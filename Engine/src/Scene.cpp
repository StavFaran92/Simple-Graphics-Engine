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
#include "SkyboxRenderer.h"
#include "Material.h"
#include "ScriptableEntity.h"
#include "PhysicsSystem.h"
#include "PhysXUtils.h"
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

int Scene::getRenderTarget() const
{
	return m_renderTargetTexture.get()->getID();
}

glm::mat4 Scene::getProjection() const
{
	return m_defaultPerspectiveProjection;
}

Entity Scene::getActiveCamera() const
{
	return m_primaryCamera;
}

//void Scene::setPrimaryEditorCamera(Entity e)
//{
//	m_primaryEditorCamera = e;
//}

void Scene::setPrimaryCamera(Entity e)
{
	m_primaryCamera = e;
}

//void Scene::setPrimaryCamera(ICamera* camera)
//{
//	m_activeCamera = camera;
//}

void Scene::onWindowResize(int width, int height)
{
        float aspect = static_cast<float>(width) / static_cast<float>(height);

        auto camView = m_registry->get().view<CameraComponent>();
        for (auto [entity, cam] : camView.each())
        {
                cam.aspect = aspect;
        }

        if (m_primaryCamera.valid() && m_primaryCamera.HasComponent<CameraComponent>())
        {
                auto& cam = m_primaryCamera.getComponent<CameraComponent>();
                m_defaultPerspectiveProjection = glm::perspective(cam.fovy, aspect, cam.znear, cam.zfar);
        }
        else
        {
                m_defaultPerspectiveProjection = glm::perspective(45.0f, aspect, 0.1f, 1000.0f);
        }

        m_renderTargetTexture = Texture::createEmptyTexture(width, height);
        m_renderTargetRBO = std::make_shared<RenderBufferObject>(width, height);

        m_renderTargetFBO->bind();
        m_renderTargetFBO->attachTexture(m_renderTargetTexture.get()->getID(), GL_COLOR_ATTACHMENT0);
        unsigned int attachments[1] = { GL_COLOR_ATTACHMENT0 };
        glDrawBuffers(1, attachments);
        m_renderTargetFBO->attachRenderBuffer(m_renderTargetRBO->GetID(), FrameBufferObject::AttachmentType::Depth_Stencil);
        m_renderTargetFBO->unbind();

        m_deferredRenderer = std::make_shared<DeferredRenderer>(m_renderTargetFBO, this);
        m_deferredRenderer->init();
        m_forwardRenderer = std::make_shared<Renderer>(m_renderTargetFBO, this);
        m_forwardRenderer->init();

        if (m_postProcessProjector)
        {
                m_postProcessProjector->init(width, height);
        }
}

void Scene::init(Context* context)
{
	m_context = context;

	m_registry = std::make_shared<SGE_Regsitry>();

	auto width = Engine::get()->getWindow()->getWidth();
	auto height = Engine::get()->getWindow()->getHeight();

	m_renderTargetFBO = std::make_shared<FrameBufferObject>();
	m_renderTargetRBO = std::make_shared<RenderBufferObject>(
		Engine::get()->getWindow()->getWidth(), 
		Engine::get()->getWindow()->getHeight());

	m_renderTargetFBO->bind();

	// Generate Texture for Position data
	m_renderTargetTexture = Texture::createEmptyTexture(width, height);
	m_renderTargetFBO->attachTexture(m_renderTargetTexture.get()->getID(), GL_COLOR_ATTACHMENT0);

	unsigned int attachments[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, attachments);

	// Create RBO and attach to FBO
	m_renderTargetFBO->attachRenderBuffer(m_renderTargetRBO->GetID(), FrameBufferObject::AttachmentType::Depth_Stencil);

	if (!m_renderTargetFBO->isComplete())
	{
		logError("FBO is not complete!");
		return;
	}

	m_renderTargetFBO->unbind();

	m_deferredRenderer = std::make_shared<DeferredRenderer>(m_renderTargetFBO, this);
	m_deferredRenderer->init();

	m_forwardRenderer = std::make_shared<Renderer>(m_renderTargetFBO, this);
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

#ifdef SGE_DEBUG
	physx::PxPvdSceneClient* pvdClient = m_PhysicsScene->getScenePvdClient();
	if (pvdClient)
	{
		pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
	}

	m_PhysicsScene->setVisualizationParameter(physx::PxVisualizationParameter::eJOINT_LOCAL_FRAMES, 1.0f);
	m_PhysicsScene->setVisualizationParameter(physx::PxVisualizationParameter::eJOINT_LIMITS, 1.0f);
#endif // SGE_DEBUG

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

	m_UIShader = Shader::createShared<Shader>(SGE_ROOT_DIR + "Resources/Engine/Shaders/UIShader.glsl");
	m_terrainShader = Shader::createShared<Shader>(SGE_ROOT_DIR + "Resources/Engine/Shaders/TerrainShader.glsl"); // todo consider move to context (or even Engine)
	m_tempOutlineShader = Shader::create<Shader>(SGE_ROOT_DIR + "Resources/Engine/Shaders/OutlineShader.glsl");

	m_uboTime = std::make_shared<UniformBufferObject>(sizeof(float));
	m_uboTime->attachToBindPoint(0);

	// Create irradiance map using created cubemap
	m_irradianceMap = Cubemap::createDefaultCubemap();

	// Create prefilter env map using created cubemap
	m_prefilterEnvMap = Cubemap::createDefaultCubemap();

	// Create BRDF look up texture
	m_BRDFIntegrationLUT = IBL::generateBRDFIntegrationLUT(this);

	m_skyboxShader = Shader::createShared<Shader>(SGE_ROOT_DIR +"Resources/Engine/Shaders/SkyboxShader.glsl");

	m_basicBox = ShapeFactory::createBox();

	//m_registry->get().on_construct<RigidBodyComponent>().connect<&Scene::onRigidBodyConstruct>(this);
	//m_registry->get().on_construct<CollisionBoxComponent>().connect<&Scene::onCollisionConstruct>(this);
	//m_registry->get().on_construct<CollisionSphereComponent>().connect<&Scene::onCollisionConstruct>(this);
	//m_registry->get().on_construct<CollisionMeshComponent>().connect<&Scene::onCollisionConstruct>(this);
	//
	//m_registry->get().on_destroy<RigidBodyComponent>().connect<&Scene::onRigidBodyDestroy>(this);
}

void Scene::update(float deltaTime)
{
	for (auto&& [entity, transform] : m_registry->get().view<Transformation>().each())
	{
		transform.update();
	}

	m_primaryCamera.getComponent<Transformation>().update();

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
		m_PhysicsScene->simulate(1 / 120.f);
		m_PhysicsScene->fetchResults(true);

		// Update kinematics
		physx::PxU32 nbDynamicActors = m_PhysicsScene->getNbActors(physx::PxActorTypeFlag::eRIGID_DYNAMIC);
		if (nbDynamicActors)
		{
			std::vector<physx::PxRigidActor*> actors(nbDynamicActors);
			m_PhysicsScene->getActors(physx::PxActorTypeFlag::eRIGID_DYNAMIC, reinterpret_cast<physx::PxActor**>(&actors[0]), nbDynamicActors);

			for (physx::PxRigidActor* actor : actors)
			{
				auto dynamicBody = static_cast<physx::PxRigidDynamic*>(actor);
				auto& flags = dynamicBody->getRigidBodyFlags();
				if (flags.isSet(physx::PxRigidBodyFlag::eKINEMATIC))
				{
					entity_id id = *(entity_id*)actor->userData;
					Entity e{ entt::entity(id), m_registry.get() };
					auto& rb = e.getComponent<RigidBodyComponent>();

					physx::PxTransform targetPose = actor->getGlobalPose();
					targetPose.p += physx::PxVec3(rb.m_targetPisition.x, rb.m_targetPisition.y, rb.m_targetPisition.z);
					targetPose.q = physx::PxQuat(physx::PxIdentity);

					if (rb.isChanged)
					{
						dynamicBody->setKinematicTarget(targetPose);
						rb.isChanged = false;
					}
				}
				else // Dynamic
				{
					entity_id id = *(entity_id*)actor->userData;
					Entity e{ entt::entity(id), m_registry.get() };
					auto& rb = e.getComponent<RigidBodyComponent>();

					if (rb.isChanged)
					{
						
						physx::PxVec3 force(rb.m_force.x, rb.m_force.y, rb.m_force.z);
						dynamicBody->addForce(force);
						rb.isChanged = false;
						rb.m_force = glm::vec3(0);
					}
				}
			}
		}

		// Retrieve Graphics transform from Physics transform
		physx::PxU32 nbActors = m_PhysicsScene->getNbActors(physx::PxActorTypeFlag::eRIGID_DYNAMIC | physx::PxActorTypeFlag::eRIGID_STATIC);
		if (nbActors)
		{
			std::vector<physx::PxRigidActor*> actors(nbActors);
			m_PhysicsScene->getActors(physx::PxActorTypeFlag::eRIGID_DYNAMIC | physx::PxActorTypeFlag::eRIGID_STATIC, reinterpret_cast<physx::PxActor**>(&actors[0]), nbActors);

			for (physx::PxRigidActor* actor : actors)
			{
				entity_id id = *(entity_id*)actor->userData;
				Entity e{ entt::entity(id), m_registry.get() };
				auto& transform = e.getComponent<Transformation>();

				physx::PxTransform pxTransform = actor->getGlobalPose();
				PhysXUtils::fromPhysXTransform(e, pxTransform, transform);
			}
		}

		for (auto&& [entity, animator, mesh] : m_registry->get().view<Animator, MeshComponent>().each())
		{
			animator.update(deltaTime);
		}
	}
}

void Scene::draw(float deltaTime)
{
	auto graphics = Engine::get()->getSubSystem<Graphics>();

	glViewport(0, 0, Engine::get()->getWindow()->getWidth(), Engine::get()->getWindow()->getHeight());
	m_deferredRenderer->clear();

	auto& primaryCamera = m_primaryCamera.getComponent<CameraComponent>();
	auto& primaryCameraTransform = m_primaryCamera.getComponent<Transformation>();


	graphics->scene = this;
	graphics->context = m_context;
	graphics->renderer = m_forwardRenderer.get();
	graphics->view = &glm::lookAt(primaryCameraTransform.getWorldPosition(), primaryCameraTransform.getWorldPosition() + primaryCamera.front, primaryCamera.up);
	graphics->projection = &m_defaultPerspectiveProjection;
	graphics->cameraPos = primaryCameraTransform.getWorldPosition();
	graphics->irradianceMap = m_irradianceMap;
	graphics->prefilterEnvMap = m_prefilterEnvMap;
	graphics->brdfLUT = m_BRDFIntegrationLUT;

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

	auto view = m_registry->get().view<MeshComponent, Transformation, RenderableComponent>();

	// TODO consider optimizing this
	std::vector<Entity> deferredRendererEntityGroup;
	std::vector<Entity> forwardRendererEntityGroup;
	auto iter = view.begin();
	while (iter != view.end())
	{
		Entity entityhandler{ *iter, m_registry.get() };

		auto& renderable = entityhandler.getComponent<RenderableComponent>();
		if (renderable.renderTechnique == RenderableComponent::RenderTechnique::Deferred)
		{
			auto& transform = entityhandler.getComponent<Transformation>();
			auto& mesh = entityhandler.getComponent<MeshComponent>();

			deferredRendererEntityGroup.push_back(entityhandler);
		}
		else if (renderable.renderTechnique == RenderableComponent::RenderTechnique::Forward)
		{
			forwardRendererEntityGroup.push_back(entityhandler);
		}

		iter++;
	}

	std::vector<Entity> builtInShaderEntityGroup;
	std::vector<Entity> customShaderEntityGroup;
	for (const auto& e : deferredRendererEntityGroup)
	{
		if (e.HasComponent<ShaderComponent>())
		{
			customShaderEntityGroup.push_back(e);
			
		}
		else
		{
			builtInShaderEntityGroup.push_back(e);
		}
	}

	

	glBindFramebuffer(GL_FRAMEBUFFER, m_deferredRenderer->getGBuffer().getID());
	m_deferredRenderer->clear();

	// Render entities with built-in shader
	graphics->entityGroup = &builtInShaderEntityGroup;
	m_deferredRenderer->renderScene(this);

	//glBindFramebuffer(GL_FRAMEBUFFER, m_deferredRenderer->getGBuffer().getID());

	// Render entities with custom shader
	for (auto& e : customShaderEntityGroup)
	{
		graphics->entity = &e;
		m_deferredRenderer->renderSceneUsingCustomShader();
	}

	const FrameBufferObject& gBuffer = m_deferredRenderer->getGBuffer();
	auto rTarget = m_forwardRenderer->getRenderTarget();

	// Bind G-Buffer as src
	glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer.getID());

	// Bind default buffer as dest
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, rTarget);

	auto width = Engine::get()->getWindow()->getWidth();
	auto height = Engine::get()->getWindow()->getHeight();

	// Copy src to dest
	glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);


	glBindFramebuffer(GL_FRAMEBUFFER, rTarget);

	graphics->entityGroup = &forwardRendererEntityGroup;

	m_forwardRenderer->renderScene(this);

	// Render skybox
	glDepthMask(GL_FALSE);
	glDepthFunc(GL_LEQUAL);
	m_skyboxShader->use();
	m_renderTargetFBO->bind();

	m_skyboxShader->setViewMatrix(*graphics->view);
	m_skyboxShader->setProjectionMatrix(*graphics->projection);

	for (auto&& [entity, skybox, transform] : 
		m_registry->get().view<SkyboxComponent, Transformation>().each())
	{
		Entity entityhandler{ entity, m_registry.get()};
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

	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	//glDisable(GL_POLYGON_OFFSET_LINE);
	

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

#if 1

	//glBindFramebuffer(GL_READ_FRAMEBUFFER, m_renderTargetFBO->getID());
	//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	//glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);

#endif 

#if 0
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_POLYGON_OFFSET_LINE);
	glPolygonOffset(-1.0, -1.0);
	for (auto&& [entity, cb] : m_registry.view<CollisionBoxComponent>().each())
	{
		Entity e{ entity, this };

		displayWireframeMesh(e, params);
	}

	for (auto&& [entity, cb] : m_registry.view<CollisionSphereComponent>().each())
	{
		Entity e{ entity, this };

		displayWireframeMesh(e, params);
	}

	for (auto&& [entity, cb] : m_registry.view<CollisionMeshComponent>().each())
	{
		Entity e{ entity, this };

		displayWireframeMesh(e, params);
	}
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_POLYGON_OFFSET_LINE);
#endif

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
	return createEntity(name);
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
	auto& primaryCamera = m_primaryCamera.getComponent<CameraComponent>();
	auto& primaryCameraTransform = m_primaryCamera.getComponent<Transformation>();

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

bool Scene::setPostProcessShader(Shader* shader)
{
	if (m_postProcessProjector)
	{
		m_postProcessProjector->setPostProcessShader(std::shared_ptr<Shader>(shader));
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

	auto physicsSystem = Engine::get()->getPhysicsSystem();

	for (auto&& [entity, rb] : m_registry->get().view<RigidBodyComponent>().each())
	{
		createActor(entity, rb);
	}

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
	}

	//for (auto&& [entity, rb] : m_registry.view<RigidBodyComponent>().each())
	//{
	//	Entity e{ entity, this };
	//	for (auto [ec, child] : e.getChildren())
	//	{
	//		if (child.valid())
	//		{
	//			auto childRigidBody = child.tryGetComponent<RigidBodyComponent>();
	//			if (!childRigidBody)
	//			{
	//				continue;
	//			}
	//			auto& transform = e.getComponent<Transformation>();
	//			auto& parentTransform = child.getComponent<Transformation>();
	//			

	//			auto joint = physx::PxFixedJointCreate(*physicsSystem->getPhysics(), (physx::PxRigidActor*)rb.simulatedBody, PhysXUtils::toPhysXTransform(parentTransform), (physx::PxRigidActor*)childRigidBody->simulatedBody, PhysXUtils::toPhysXTransform(transform));
	//			joint->setConstraintFlags(physx::PxConstraintFlag::eVISUALIZATION);
	//		}
	//	}
	//}

	//for (auto&& [entity, rb] : m_registry.view<RigidBodyComponent>().each())
	//{
	//	Entity e{ entity, this };

	//	auto& transform = e.getComponent<Transformation>();
	//	auto scale = transform.getScale();
 //		auto body = physicsSystem->createRigidBody(transform, rb.type, rb.mass);

	//	if (e.HasComponent<CollisionBoxComponent>())
	//	{
	//		auto& collider = e.getComponent<CollisionBoxComponent>();
	//		physx::PxShape* shape = physicsSystem->createBoxShape(collider.halfExtent * scale.x, collider.halfExtent * scale.y, collider.halfExtent * scale.z);
	//		body->attachShape(*shape);
	//		shape->release();
	//	}
	//	else if (e.HasComponent<CollisionSphereComponent>())
	//	{
	//		auto& collider = e.getComponent<CollisionSphereComponent>();
	//		physx::PxShape* shape = physicsSystem->createSphereShape(collider.radius * std::max(std::max(scale.x, scale.y), scale.z));
	//		body->attachShape(*shape);
	//		shape->release();
	//	}
	//	else if (e.HasComponent<CollisionConvexMeshComponent>())
	//	{
	//		//auto& mesh = e.getComponent<Mesh>();
	//		std::vector<glm::vec3> pos = { {0,0,0}, {0,1,0}, {1,0,0} };
	//		physx::PxShape* shape = physicsSystem->createConvexMeshShape(pos);
	//		//physx::PxShape* shape = physicsSystem->createConvexMeshShape(*mesh.getPositions());
	//		body->attachShape(*shape);
	//		shape->release();
	//	}


	//	assert(body);

	//	m_PhysicsScene->addActor(*body);
	//	rb.simulatedBody = body;
	//}

	m_isSimulationActive = true;
}

void Scene::createActor(entt::entity entity, RigidBodyComponent& rb)
{
	Entity e{ entity, m_registry.get() };

	auto& transform = e.getComponent<Transformation>();
	auto body = Engine::get()->getPhysicsSystem()->createRigidBody(transform, rb);

	createShape(Engine::get()->getPhysicsSystem(), body, e, true);

	m_PhysicsScene->addActor(*body);
	entity_id* id = new entity_id(e.handlerID());
	body->userData = (void*)id;
	rb.simulatedBody = (void*)body;
}

void Scene::removeActor(entt::entity entity, RigidBodyComponent& rb)
{
	Entity e{ entity, m_registry.get() };

	auto& rBody = e.getComponent<RigidBodyComponent>();
	m_PhysicsScene->removeActor(*(physx::PxRigidActor*)rBody.simulatedBody);
}

void Scene::stopSimulation()
{
	if (!m_isSimulationActive)
	{
		return;
	}

	for (auto&& [entity, rb] : m_registry->get().view<RigidBodyComponent>().each())
	{
		removeActor(entity, rb);
	}

	for (auto&& [entity, nsc] : m_registry->get().view<NativeScriptComponent>().each())
	{
		nsc.script->onDestroy();
	}

	getRegistry().getRegistry().clear();

	Archiver::deserializeScene(m_serializedScene, *this);


	m_isSimulationActive = false;
}

physx::PxScene* Scene::getPhysicsScene() const
{
	return m_PhysicsScene;
}

bool Scene::isSimulationActive() const
{
	return m_isSimulationActive;
}

void Scene::createShape(PhysicsSystem* physicsSystem, physx::PxRigidActor* body, Entity e, bool recursive)
{
	physx::PxShape* shape = nullptr;
	auto& transform = e.getComponent<Transformation>();
	auto scale = transform.getWorldScale();

	if (e.HasComponent<CollisionBoxComponent>())
	{
		auto& collider = e.getComponent<CollisionBoxComponent>();
		shape = physicsSystem->createBoxShape(collider.halfExtent * scale.x, collider.halfExtent * scale.y, collider.halfExtent * scale.z);

		Physics::LayerMask mask = collider.layerMask;

		physx::PxFilterData filterData;
		filterData.word0 = mask;

		shape->setQueryFilterData(filterData);
	}
	else if (e.HasComponent<CollisionSphereComponent>())
	{
		auto& collider = e.getComponent<CollisionSphereComponent>();
		shape = physicsSystem->createSphereShape(collider.radius * std::max(std::max(scale.x, scale.y), scale.z));

		Physics::LayerMask mask = collider.layerMask;

		physx::PxFilterData filterData;
		filterData.word0 = mask;

		shape->setQueryFilterData(filterData);
	}
	else if (e.HasComponent<CollisionMeshComponent>())
	{
		auto collisionMeshComponent = e.getComponent<CollisionMeshComponent>();
		const std::vector<glm::vec3>& apos = collisionMeshComponent.mesh.get()->getPositions();
		shape = physicsSystem->createConvexMeshShape(apos);

		Physics::LayerMask mask = collisionMeshComponent.layerMask;

		physx::PxFilterData filterData;
		filterData.word0 = mask;

		shape->setQueryFilterData(filterData);
	}

	if (shape)
	{
		
		auto translation = transform.getWorldPosition();
		auto orientation = transform.getWorldRotation();

		physx::PxVec3 pxTranslation(translation.x, translation.y, translation.z);
		pxTranslation -= body->getGlobalPose().p;
		physx::PxQuat pxRotation(orientation.x, orientation.y, orientation.z, orientation.w);
		pxRotation *= body->getGlobalPose().q.getConjugate();

		auto physxTransform = physx::PxTransform(pxTranslation, pxRotation);

		//auto physxTransform = PhysXUtils::toPhysXTransform(transform);
		shape->setLocalPose(physxTransform);
		body->attachShape(*shape);
		shape->release();
	}

	if (recursive)
	{
		for (auto [eid, child] : e.getChildren())
		{
			createShape(physicsSystem, body, child, true);
		}
	}
}

void Scene::onRigidBodyConstruct(entt::registry& registry, entt::entity entity)
{
	// if add rigid body
		// create actor
			// create shape recursive
	// if add collision
		// look for rigid body in hierarchy
			// if found add shape to body (not recursive)

	if (m_isSimulationActive)
	{
		Entity e(entity, m_registry.get());
		auto physicsSystem = Engine::get()->getPhysicsSystem();
		auto& rb = e.getComponent<RigidBodyComponent>();
		createActor(entity, rb);
	}
}

void Scene::onRigidBodyDestroy(entt::registry& registry, entt::entity entity)
{
	if (m_isSimulationActive)
	{
		Entity e(entity, m_registry.get());
		auto physicsSystem = Engine::get()->getPhysicsSystem();
		auto& rb = e.getComponent<RigidBodyComponent>();
		removeActor(entity, rb);
	}
}

void Scene::onCollisionConstruct(entt::registry& registry, entt::entity entity)
{
	if (m_isSimulationActive)
	{
		Entity e(entity, m_registry.get());
		auto rb = e.tryGetComponentInParent<RigidBodyComponent>(true);
		if (rb)
		{
			auto physicsSystem = Engine::get()->getPhysicsSystem();
			auto body = (physx::PxRigidActor*)rb->simulatedBody;
			createShape(physicsSystem, body, e, false);
		}
	}
}
