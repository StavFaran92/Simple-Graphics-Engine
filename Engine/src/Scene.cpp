#include "Scene.h"

#include "Skybox.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "Engine.h"
#include "ICamera.h"
#include "SkyboxRenderer.h"
#include "PostProcessProjector.h"
#include "CoroutineSystem.h"
#include "Logger.h"
#include "PhongShader.h"
#include "PickingShader.h"
#include "Context.h"
#include "Window.h"
#include "ObjectPicker.h"
#include "Configurations.h"
#include "Shader.h"
#include "Entity.h"
#include "Transformation.h"
#include "Mesh.h"
#include "RenderableComponent.h"
#include "Component.h"
#include "Shader.h"
#include "Material.h"
#include "ScriptableEntity.h"
#include "PhysicsSystem.h"
#include "Box.h"
#include "EditorCamera.h"
#include "ShadowSystem.h"
#include "LightSystem.h"
#include "Engine.h"
#include "TimeManager.h"
#include "UniformBufferObject.h"
#include "DeferredRenderer.h"
#include "Random.h"
#include "ShapeFactory.h"
#include <GL/glew.h>
#include "EquirectangularToCubemapConverter.h"
#include "CommonTextures.h"
#include "RenderCommand.h"
#include "IBL.h"
#include "Registry.h"
#include "Physics.h"
#include "Archiver.h"
#include "Animator.h"
#include "Terrain.h"
#include "AABB.h"
#include "Frustum.h"
#include "MeshCollection.h"
#include "Graphics.h"
#include "DebugHelper.h"
#include "Cubemap.h"
#include "RenderView.h"
#include "GameLayer.h"
#include "EventLayerStack.h"

void cameraCalculateOrientation(Transformation& transform, CameraComponent& cameraComponent)
{
	auto quat = transform.getWorldRotation();

	glm::vec3& front = cameraComponent.front;
	glm::vec3& right = cameraComponent.right;
	glm::vec3& up = cameraComponent.up;
	glm::vec3& center = cameraComponent.center;

	front = quat * glm::vec3(0.0f, 0.0f, -1.0f); // Forward (Z-axis negative in OpenGL)
	right = quat * glm::vec3(1.0f, 0.0f, 0.0f);  // Right (X-axis)

	front = glm::normalize(front);
	center = transform.getLocalPosition() + front;

	right = glm::normalize(glm::cross(front, { 0,1,0 }));
	up = glm::normalize(glm::cross(right, front));
}

void Scene::displayWireframeMesh(Entity e)
{
	auto graphics = Engine::get()->getSubSystem<Graphics>();

	for (auto& mesh : e.tryGetComponent<MeshComponent>()->mesh.get()->getMeshes())
	{
		graphics->entity = &e;
		graphics->shader = m_tempOutlineShader;
		graphics->mesh = mesh.get();
		auto tempModel = e.getComponent<Transformation>().getWorldTransformation();
		graphics->model = &tempModel;

		m_deferredRenderer->render();
	}
}

void Scene::setIBLData(Resource<Texture> irradianceMap, Resource<Texture> prefilterEnvMap)
{
	m_irradianceMap = irradianceMap;
	m_prefilterEnvMap = prefilterEnvMap;
}

glm::mat4 Scene::getProjection() const
{
	return m_defaultPerspectiveProjection;
}

Entity Scene::getActiveCamera() const
{
	assert(m_renderViews.size() > 0);
	return m_renderViews[0]->getCamera();
}

void Scene::setPrimaryCamera(Entity e)
{
	assert(m_renderViews.size() > 0);
	m_renderViews[0]->setCamera(e);
}

void Scene::init(Context* context)
{
	m_context = context;

	m_registry = std::make_shared<SGE_Regsitry>();

	auto width = Engine::get()->getWindow()->getWidth();
	auto height = Engine::get()->getWindow()->getHeight();

	gameEventLayer = std::make_shared<GameLayer>();
	Engine::get()->getEventLayerStack()->addLayer(gameEventLayer);

	m_deferredRenderer = std::make_shared<DeferredRenderer>(this);
	m_deferredRenderer->init();

	m_forwardRenderer = std::make_shared<Renderer>(this);
	m_forwardRenderer->init();

	//m_skyboxRenderer = std::make_shared<SkyboxRenderer>(*m_renderer.get());
	//m_gpuInstancingRenderer = std::make_shared<GpuInstancingRenderer>();
	//m_objectSelection = std::make_shared<ObjectSelection>(m_context, this);
	//m_objectPicker = std::make_shared<ObjectPicker>();
	//if (!m_objectPicker->init())
	//{
	//	logError("Object picker failed to init!");
	//}

	m_postProcessProjector = std::make_shared<PostProcessProjector>(this);
	if (!m_postProcessProjector->init(width, height))
	{
		logError("Post process projector failed to init!");
	}
	m_postProcessProjector->setEnabled(false);

	m_coroutineManager = std::make_shared<CoroutineSystem>();

	m_PhysicsScene = Engine::get()->getPhysicsSystem()->createScene();

	m_shadowSystem = std::make_shared<ShadowSystem>(m_context, this);
	if (!m_shadowSystem->init())
	{
		logError("Shadow System init failed!");
	} 


	m_lightSystem = std::make_shared<LightSystem>(m_context, this);
	if (!m_lightSystem->init())
	{
		logError("Light System init failed!");
	}

	m_defaultPerspectiveProjection = glm::perspective(45.0f, (float)4 / 3, 0.1f, 1000.0f);

	m_defaultUIProjection = glm::ortho(0.0f, (float)Engine::get()->getWindow()->getWidth(), (float)Engine::get()->getWindow()->getHeight(), 0.0f, -1.0f, 1.0f);
	
	m_quadUI = ShapeFactory::createQuad(&getRegistry());
	m_quadUI.RemoveComponent<RenderableComponent>();
	m_quadUI.RemoveComponent<ObjectComponent>();

	m_UIShader = Shader::create(SGE_ROOT_DIR + "Resources/Engine/Shaders/UIShader.glsl");
	m_terrainShader = Shader::create(SGE_ROOT_DIR + "Resources/Engine/Shaders/TerrainShader.glsl"); // todo consider move to context (or even Engine)
	m_tempOutlineShader = Shader::create(SGE_ROOT_DIR + "Resources/Engine/Shaders/OutlineShader.glsl");

	m_uboTime = std::make_shared<UniformBufferObject>(sizeof(float));
	m_uboTime->attachToBindPoint(0);

	// Create irradiance map using created cubemap
	m_irradianceMap = Cubemap::createDefaultCubemap();

	// Create prefilter env map using created cubemap
	m_prefilterEnvMap = Cubemap::createDefaultCubemap();

	// Create BRDF look up texture
	m_BRDFIntegrationLUT = IBL::generateBRDFIntegrationLUT(this);

	m_skyboxShader = Shader::create(SGE_ROOT_DIR +"Resources/Engine/Shaders/SkyboxShader.glsl");

	m_basicBox = ShapeFactory::createBox();

	addRenderView(0, 0, Engine::get()->getWindow()->getWidth(), Engine::get()->getWindow()->getHeight(), Entity::EmptyEntity);
}

void Scene::update(float deltaTime)
{
	for (auto&& [entity, transform] : m_registry->get().view<Transformation>().each())
	{
		transform.update();
	}

	getActiveCamera().getComponent<Transformation>().update();

	if (m_isSimulationActive)
	{

		// Advance all coroutines
		auto coroutines = m_coroutineManager->getAllCoroutines();
		for (int i = 0; i < coroutines.size(); i++)
		{
			if ((*coroutines[i])(deltaTime))
			{
				m_coroutineManager->removeCoroutine(i);
			}
		}

		// Run all User Scriptable Entities scripts
		for (auto&& [entity, nsc] : m_registry->get().view<NativeScriptComponent>().each())
		{
			if (!nsc.script)
			{
				logWarning("Native Script cannot be Null, did you forget to call Bind()?");
				continue;
			}

			nsc.script->onUpdate(deltaTime);
		}

		// Physics
		Engine::get()->getPhysicsSystem()->update(this, deltaTime);

		for (auto&& [entity, animator, mesh] : m_registry->get().view<Animator, MeshComponent>().each())
		{
			animator.update(deltaTime);
		}
	}
}

void Scene::draw(float deltaTime)
{
	auto graphics = Engine::get()->getSubSystem<Graphics>();

	for (auto& renderView : m_renderViews)
	{
		auto viewport = renderView->getViewport();

		renderView->bind();
		RenderCommand::clear();

		RenderCommand::setViewport(viewport.x, viewport.y, viewport.w, viewport.h);

		const Entity& camera = renderView->getCamera();

		auto& primaryCamera = camera.getComponent<CameraComponent>();
		auto& primaryCameraTransform = camera.getComponent<Transformation>();

		cameraCalculateOrientation(primaryCameraTransform, primaryCamera);

		graphics->scene = this;
		graphics->context = m_context;
		graphics->renderer = m_forwardRenderer.get();
		graphics->view = &glm::lookAt(primaryCameraTransform.getWorldPosition(), primaryCameraTransform.getWorldPosition() + primaryCamera.front, primaryCamera.up);
		graphics->projection = &m_defaultPerspectiveProjection;
		graphics->cameraPos = primaryCameraTransform.getWorldPosition();
		graphics->irradianceMap = m_irradianceMap;
		graphics->prefilterEnvMap = m_prefilterEnvMap;
		graphics->brdfLUT = m_BRDFIntegrationLUT;
		graphics->renderView = renderView;

		m_shadowSystem->renderToDepthMap();

		graphics->lightSpaceMatrix = m_shadowSystem->getLightSpaceMat();
		graphics->shadowMap = m_shadowSystem->getShadowMap();

		// PRE Render Phase
		for (const auto& cb : m_renderCallbacks[RenderPhase::PRE_RENDER_BEGIN])
		{
			cb();
		}

		for (const auto& cb : m_renderCallbacks[RenderPhase::PRE_RENDER_END])
		{
			cb();
		}

		// Set time elapsed
		auto elapsed = (float)Engine::get()->getTimeManager()->getElapsedTime(TimeManager::Duration::MilliSeconds) / 1000;

		m_uboTime->bind();
		m_uboTime->setData(0, sizeof(float), &elapsed);
		m_uboTime->unbind();

		Frustum frustum(primaryCameraTransform.getWorldPosition(),
			primaryCamera.front,
			primaryCamera.up,
			primaryCamera.right,
			primaryCamera.aspect,
			primaryCamera.fovy,
			primaryCamera.znear,
			primaryCamera.zfar);

		graphics->frustum = &frustum;

		RenderCommand::setViewport(viewport.x, viewport.y, viewport.w, viewport.h);

		m_deferredRenderer->renderScene(this);

		unsigned int srcID = m_deferredRenderer->getGBuffer().getID();
		unsigned int dstID = graphics->renderView->getRenderTargetFrameBufferID();

		RenderCommand::copyFrameBufferData(srcID, dstID, RenderCommand::BufferBit::DEPTH_BUFFER_BIT);

		m_forwardRenderer->renderScene(this);

		m_forwardRenderer->renderSceneUsingCustomShader(this);

		// Render skybox
		glDepthMask(GL_FALSE);
		glDepthFunc(GL_LEQUAL);
		m_skyboxShader->use();
		renderView->bind();

		m_skyboxShader->setViewMatrix(*graphics->view);
		m_skyboxShader->setProjectionMatrix(*graphics->projection);

		for (auto&& [entity, skybox, transform] :
			m_registry->get().view<SkyboxComponent, Transformation>().each())
		{
			Entity entityhandler{ entity, m_registry.get() };
			graphics->entity = &entityhandler;
			graphics->mesh = m_basicBox.get()->getPrimaryMesh().get(); // todo can be optimized using a single mesh
			graphics->model = &transform.getWorldTransformation();

			if (skybox.skyboxImage.isEmpty()) continue;

			skybox.skyboxImage.get()->bind();
			skybox.skyboxImage.get()->setSlot(0);

			auto vao = graphics->mesh->getVAO();
			RenderCommand::draw(vao);
		}
		glDepthMask(GL_TRUE);
		glDepthFunc(GL_LESS);

		m_terrainShader->use();

		m_terrainShader->setUniformValue("view", *graphics->view);
		m_terrainShader->setUniformValue("projection", *graphics->projection);

		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		//glEnable(GL_POLYGON_OFFSET_LINE);
		//glPolygonOffset(-1.0, -1.0);

		// Render terrain
		for (auto&& [entity, terrain, transform] : m_registry->get().view<Terrain, Transformation>().each())
		{
			m_terrainShader->setUniformValue("scale", terrain.getScale());
			m_terrainShader->setUniformValue("model", transform.getWorldTransformation());
			m_terrainShader->setUniformValue("width", terrain.getWidth());
			m_terrainShader->setUniformValue("height", terrain.getHeight());
			Resource<Texture> heightmap = terrain.getHeightmap();

			if (heightmap.isEmpty())
				continue;

			heightmap.get()->bind();
			heightmap.get()->setSlot(0);
			m_terrainShader->setUniformValue("heightMap", 0);

			int textureCount = terrain.getTextureCount();
			m_terrainShader->setUniformValue("textureCount", textureCount);

			for (int i = 0; i < textureCount; i++)
			{
				auto texture = terrain.getTexture(i);
				texture.get()->setSlot(i + 1);
				texture.get()->bind();
				m_terrainShader->setUniformValue("texture_" + std::to_string(i), i + 1);

				auto textureBlend = terrain.getTextureBlend(i);
				m_terrainShader->setUniformValue("textureBlend[" + std::to_string(i) + "]", textureBlend);

				glm::vec2 textureScale = terrain.getTextureScale(i);
				m_terrainShader->setUniformValue("textureScale[" + std::to_string(i) + "]", textureScale);
			}

			auto vao = terrain.getMesh().get()->getPrimaryMesh()->getVAO();
			RenderCommand::drawPatches(vao);
		}

		

		// Render Post Process effects
		for (auto&& [entity, postProcess, shader] : m_registry->get().view<PostProcessComponent, ShaderComponent>().each())
		{
			Resource<Texture> renderTargetTexture = graphics->renderView->getRenderTargetTexture();
			renderView->swapToAdditionalTarget();
			renderView->bind();
			RenderCommand::clear();
			glDisable(GL_DEPTH_TEST);
			// TODO assert post process shader

			// bind shader
			shader.m_customShader->use();

			// read texture from graphics FBO
			shader.m_customShader->setTextureInShader(renderTargetTexture, "MainTexture", 0); //todo check slot

			shader.m_customShader->setModelMatrix(glm::mat4(1.0));
			shader.m_customShader->setViewMatrix(*graphics->view);
			shader.m_customShader->setProjectionMatrix(*graphics->projection);

			auto viewport = renderView->getViewport();
			shader.m_customShader->setUniformValue("screenSize", glm::vec2(viewport.w, viewport.h));

			shader.m_customShader->setUniformValue("cameraPos", graphics->cameraPos);
			shader.m_customShader->setUniformValue("cameraLookAt", primaryCamera.front);

			// bind mesh
			auto vao = m_basicBox.get()->getPrimaryMesh().get()->getVAO();
			//auto vao = m_quadUI.getComponent<MeshComponent>().mesh.get()->getPrimaryMesh()->getVAO(); //todo change, we start off with a quad

			// in frag shader i need access to mesh extentes & main texture -> set uniforms

			// draw
			RenderCommand::draw(vao);

			renderView->swapBackToMainTarget();
			renderView->bind();
			glEnable(GL_DEPTH_TEST);
		}

		

		// Render UI
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		m_UIShader->use();
		m_UIShader->setProjectionMatrix(m_defaultUIProjection);
		auto vao = m_quadUI.getComponent<MeshComponent>().mesh.get()->getPrimaryMesh()->getVAO();

		for (auto&& [entity, image] : m_registry->get().view<ImageComponent>().each())
		{
			Entity entityhandler{ entity, m_registry.get() };
			graphics->entity = &entityhandler;
			image.image.get()->bind();
			image.image.get()->setSlot(0);

			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, glm::vec3(image.position, 0.0f));

			model = glm::translate(model, glm::vec3(0.5f * image.size.x, 0.5f * image.size.y, 0.0f));
			model = glm::rotate(model, glm::radians(image.rotate), glm::vec3(0.0f, 0.0f, 1.0f));
			model = glm::translate(model, glm::vec3(-0.5f * image.size.x, -0.5f * image.size.y, 0.0f));

			model = glm::scale(model, glm::vec3(image.size, 1.0f));

			m_UIShader->setUniformValue("model", model);

			RenderCommand::draw(vao);
		}

		glDisable(GL_BLEND);

	}

	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	//glDisable(GL_POLYGON_OFFSET_LINE);

	//auto& debug = DebugHelper::getInstance();

	//static float offsetX = 0.0;
	//static float offsetY = 0.0;

	//offsetX = fmod(offsetX + .01f, 1.0f);
	//offsetY = fmod(offsetY + .01f, 1.0f);

	//for (auto& [e, terrain] : Engine::get()->getContext()->getActiveScene()->getRegistry().getRegistry().view<Terrain>().each())
	//{

	//	for (float i = -terrain.getWidth() / 2 + offsetX; i < terrain.getWidth() / 2; i++)
	//	{
	//		for (float j = -terrain.getHeight() / 2 + offsetY; j < terrain.getHeight() / 2; j++)
	//		{
	//			float height = terrain.getHeightAtPoint(i, j);
	//			debug.drawPoint({ i, height, j });
	//		}
	//	}
	//}
	



	for (const auto& cb : m_renderCallbacks[RenderPhase::POST_RENDER_BEGIN])
	{
		cb();
	}

	for (const auto& cb : m_renderCallbacks[RenderPhase::POST_RENDER_END])
	{
		cb();
	}
}

Scene::RenderCallback* Scene::addRenderCallback(RenderPhase renderPhase, RenderCallback renderCallback)
{
	if (!renderCallback)
	{
		logError("render callback cannot be null.");
		return 0;
	}

	m_renderCallbacks[renderPhase].push_back(std::move(renderCallback));
	return &m_renderCallbacks[renderPhase].back();
}

void Scene::removeRenderCallback(RenderCallback* callback)
{
	for (auto& pair : m_renderCallbacks) {
		auto& vector = pair.second;
		vector.erase(
			std::remove_if(vector.begin(), vector.end(),
				[callback](const auto& existingCallback) { return &existingCallback == callback; }),
			vector.end()
		);
	}
}

SGE_Regsitry& Scene::getRegistry()
{
	return *m_registry.get();
}

const SGE_Regsitry& Scene::getRegistry() const
{
	return *m_registry.get();
}

Entity Scene::createEntity()
{
	std::string name = "temp";
	Entity e = createEntity(name);
	return e;
}

Entity Scene::createEntity(const std::string& name)
{
	return m_registry->createEntity(name);
}

void Scene::removeEntity(const Entity& e)
{
	m_registry->removeEntity(e);
}

glm::mat4 Scene::getActiveCameraView() const
{
	auto& primaryCamera = getActiveCamera().getComponent<CameraComponent>();
	auto& primaryCameraTransform = getActiveCamera().getComponent<Transformation>();

	return glm::lookAt(primaryCameraTransform.getWorldPosition(), primaryCameraTransform.getWorldPosition() + primaryCamera.front, primaryCamera.up);
}


void Scene::clear()
{
	m_registry->get().clear();
}

Scene::Scene(Context* context)
{
	init(context);
}

std::shared_ptr<IRenderer> Scene::getRenderer() const
{
	return m_deferredRenderer;
}

void Scene::close()
{
	clear();
}

void Scene::setPostProcess(bool value)
{
	m_postProcessProjector->setEnabled(value);
}

//bool Scene::isSelected(uint32_t id) const
//{
//	if (!m_isObjectSelectionEnabled)
//	{
//		logWarning("Object selection isn't enabled for this scene.");
//		return false;
//	}
//
//	return m_objectSelection->isObjectSelected(id);
//}

bool Scene::setPostProcessShader(Resource<Shader> shader)
{
	if (m_postProcessProjector)
	{
		m_postProcessProjector->setPostProcessShader(shader);
		return true;
	}
	return false;
}

void Scene::addCoroutine(const std::function<bool(float)>& coroutine)
{
	m_coroutineManager->addCoroutine(coroutine);
}

//void Scene::removeCoroutine(std::function<bool(float)>* coroutine)
//{
//	m_coroutineManager->removeCoroutine(coroutine);
//}

Entity Scene::getEntityByName(const std::string& name) const
{
	// TODO optimize this using hashmap
	for (auto& [e, obj] : m_registry->getRegistry().view<ObjectComponent>().each())
	{
		if (name == obj.name)
		{
			return Entity(e, m_registry.get());
		}
	}

	return Entity::EmptyEntity;
}

void Scene::startSimulation()
{
	if (m_isSimulationActive)
	{
		logWarning("Simulation already active.");
		return;
	}

	m_serializedScene = Archiver::serializeScene(this);

	Engine::get()->getPhysicsSystem()->startScenePhysics(this);

	// Run all User Scriptable Entities scripts
	for (auto&& [entity, nsc] : m_registry->get().view<NativeScriptComponent>().each())
	{
		if (!nsc.script)
		{
			logWarning("Native Script cannot be Null, did you forget to call Bind()?");
			continue;
		}

		nsc.script->entity = Entity(entity, &getRegistry());
		nsc.script->onCreate();

		gameEventLayer->subscribe(nsc.script);
	}

	m_isSimulationActive = true;
}

void Scene::stopSimulation()
{
	if (!m_isSimulationActive)
	{
		return;
	}

	Engine::get()->getPhysicsSystem()->stopScenePhysics(this);

	for (auto&& [entity, nsc] : m_registry->get().view<NativeScriptComponent>().each())
	{
		nsc.script->onDestroy();
		gameEventLayer->unsubscribe(nsc.script);
	}

	getRegistry().getRegistry().clear();

	Archiver::deserializeScene(m_serializedScene, *this);


	m_isSimulationActive = false;
}

physx::PxScene* Scene::getPhysicsScene() const
{
	return m_PhysicsScene;
}

unsigned int Scene::addRenderView(int x, int y, int w, int h, const Entity& e)
{
	// todo maybe use map here?
	unsigned int id = m_renderViews.size();
	m_renderViews.push_back(std::make_shared<RenderView>(Viewport{x, y, w, h}, e));
	return id;
}

unsigned int Scene::getRenderTargetFrameBufferID(unsigned int id) const
{
	assert(id < m_renderViews.size());

	return m_renderViews[id]->getRenderTargetFrameBufferID();
}

unsigned int Scene::getRenderTargetTextureID(unsigned int id) const
{
	assert(id < m_renderViews.size());

	return m_renderViews[id]->getRenderTargetTextureID();
}

bool Scene::isSimulationActive() const
{
	return m_isSimulationActive;
}