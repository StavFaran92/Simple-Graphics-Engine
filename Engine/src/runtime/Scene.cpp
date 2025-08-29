#include "runtime/Scene.h"

#include "systems/Skybox.h"
#include "lights/DirectionalLight.h"
#include "lights/PointLight.h"
#include "core/Engine.h"
#include "camera/ICamera.h"
#include "render/SkyboxRenderer.h"
#include "render/PostProcessProjector.h"
#include "core/CoroutineSystem.h"
#include "core/Logger.h"
#include "render/PhongShader.h"
#include "render/PickingShader.h"
#include "runtime/Context.h"
#include "core/Window.h"
#include "systems/ObjectPicker.h"
#include "core/Configurations.h"
#include "render/Shader.h"
#include "runtime/Entity.h"
#include "component/Transformation.h"
#include "geometry/Mesh.h"
#include "component/RenderableComponent.h"
#include "component/Component.h"
#include "render/Shader.h"
#include "render/Material.h"
#include "component/ScriptableEntity.h"
#include "physics/PhysicsSystem.h"
#include "geometry/Box.h"
#include "systems/ShadowSystem.h"
#include "lights/LightSystem.h"
#include "core/Engine.h"
#include "systems/TimeManager.h"
#include "render/UniformBufferObject.h"
#include "render/DeferredRenderer.h"
#include "core/Random.h"
#include "geometry/ShapeFactory.h"
#include <GL/glew.h>
#include "utils/EquirectangularToCubemapConverter.h"
#include "systems/CommonTextures.h"
#include "render/RenderCommand.h"
#include "glm/ext.hpp"
#include "render/IBL.h"
#include "core/Registry.h"
#include "physics/Physics.h"
#include "serialize/Archiver.h"
#include "animation/Animator.h"
#include "component/Terrain.h"
#include "geometry/AABB.h"
#include "geometry/Frustum.h"
#include "geometry/MeshCollection.h"
#include "render/Graphics.h"
#include "utils/DebugHelper.h"
#include "texture/Cubemap.h"
#include "render/RenderView.h"
#include "core/GameLayer.h"
#include "core/EventSystem.h"
#include "core/EngineConfig.h"
#include "geometry/WireframeGrid.h"
#include "systems/BuiltInMeshes.h"
#include "component/FoliageComponent.h"
#include "systems/FoliageSystem.h"
#include "component/CameraComponent.h"
#include "component/MeshComponent.h"
#include "component/ShaderComponent.h"
#include "component/ObjectComponent.h"
#include "component/SkyboxComponent.h"
#include "component/NativeScriptComponent.h"
#include "component/ImageComponent.h"

struct PlaneGPU {
	glm::vec3 normal;
	float d;
};

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

Entity Scene::getGameCamera() const
{
	assert(m_renderViews.size() > 0);
	return getRenderView("Game View")->getCamera();
}

void Scene::setGameCamera(Entity e)
{
	assert(m_renderViews.size() > 0);
	getRenderView("Game View")->setCamera(e);
}

void Scene::bindScriptToLayer(entt::registry& reg, entt::entity entity) 
{
	auto& script = reg.get<ScriptableEntity>(entity);
	script.eventHandler = Engine::get()->getEventSystem()->bindToLayer(gameEventLayer->name);
}

void Scene::init(Context* context)
{
	m_context = context;

	m_registry = std::make_shared<SGE_Regsitry>();

	auto width = Engine::get()->getWindow()->getWidth();
	auto height = Engine::get()->getWindow()->getHeight();

	gameEventLayer = std::make_shared<GameLayer>();
	gameEventLayer->setEnabled(false);
	Engine::get()->getEventSystem()->pushLayer(gameEventLayer);

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

	m_UIShader = Shader::loadTransient(SGE_ROOT_DIR + "Resources/Engine/Shaders/UIShader.glsl");
	m_terrainShader = Shader::loadTransient(SGE_ROOT_DIR + "Resources/Engine/Shaders/TerrainShader.glsl"); // todo consider move to context (or even Engine)
	m_tempOutlineShader = Shader::loadTransient(SGE_ROOT_DIR + "Resources/Engine/Shaders/OutlineShader.glsl");

	m_uboTime = std::make_shared<UniformBufferObject>(sizeof(float));
	m_uboTime->attachToBindPoint(0);

	// Create irradiance map using created cubemap
	m_irradianceMap = Cubemap::createDefaultCubemap();

	// Create prefilter env map using created cubemap
	m_prefilterEnvMap = Cubemap::createDefaultCubemap();

	// Create BRDF look up texture
	m_BRDFIntegrationLUT = IBL::generateBRDFIntegrationLUT(this);

	m_skyboxShader = Shader::loadTransient(SGE_ROOT_DIR + "Resources/Engine/Shaders/SkyboxShader.glsl");

	m_basicBox = Engine::get()->getBuiltInMeshes()->getMesh(BuiltInMeshes::MeshType::BOX);

	addRenderView("Game View", 0, 0, Engine::get()->getWindow()->getWidth(), Engine::get()->getWindow()->getHeight(), Entity::EmptyEntity);

	m_registry->getRegistry().on_construct<ScriptableEntity>().connect<&Scene::bindScriptToLayer>(this);

	m_highlightRenderView = std::make_shared<RenderView>(Viewport{ 0, 0, Engine::get()->getWindow()->getWidth(), Engine::get()->getWindow()->getHeight() }, Entity::EmptyEntity);

	m_highlightMaskShader = Shader::loadTransient(SGE_ROOT_DIR + "Resources/Engine/Shaders/HighlighMaskShader.glsl");
	m_highlightEdgeDetectionShader = Shader::createOverrideShader("HighlightEdgeDetectionShader", SGE_ROOT_DIR + "Resources/Engine/Shaders/HighlightEdgeDetectionShader.glsl", ShaderOverride::PostProcess, true);
	m_highlightMergeShader = Shader::createOverrideShader("HighlightMergeShader", SGE_ROOT_DIR + "Resources/Engine/Shaders/HighlightMergeShader.glsl", ShaderOverride::PostProcess, true);

	m_wireframeGrid = std::make_shared<WireframeGrid>();

	//std::vector<GLuint> data = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 }; // sum = 45

	//glGenBuffers(1, &ssbo);
	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
	//glBufferData(GL_SHADER_STORAGE_BUFFER, data.size() * sizeof(GLuint), data.data(), GL_DYNAMIC_DRAW);
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo); // Binding = 0


	//m_sampleComputeShader = Shader::create(SGE_ROOT_DIR + "Resources/Engine/Shaders/SampleComputeShader.glsl");

	//m_sampleComputeShader->use();
	//glDispatchCompute((GLuint)data.size(), 1, 1);
	//glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	
	//// Read result
	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
	//GLuint* ptr = (GLuint*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(GLuint), GL_MAP_READ_BIT);
	//GLuint result = ptr[0];
	//glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	//logInfo("Sum is: {}", result);
}

void Scene::update(float deltaTime)
{
	for (auto&& [entity, transform] : m_registry->get().view<Transformation>().each())
	{
		transform.update();
	}

	getGameCamera().getComponent<Transformation>().update();

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

			try
			{
				nsc.script->onUpdate(deltaTime);
			}
			catch (const std::exception& e)
			{
				logError("Native Script Error occured: {}" , e.what());
			}
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

	for (auto& [rName, renderView] : m_renderViews)
	{
		auto viewport = renderView->getViewport();

		renderView->bind();

		glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, renderView->getName().c_str());

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

		if(Engine::get()->getConfig().renderConfig.renderShadowMap)
		{
			glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Generate Shadow Map");
			m_shadowSystem->renderToDepthMap();
			glPopDebugGroup();
		}

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

		glEnable(GL_DEPTH_TEST);
		graphics->renderView->bind();

		if (Engine::get()->getConfig().renderConfig.renderDeferredPass)
		{
			glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Deferred Renderer pass");
			m_deferredRenderer->renderScene(this);
			unsigned int srcID = m_deferredRenderer->getGBuffer().getID();
			unsigned int dstID = graphics->renderView->getRenderTargetFrameBufferID();
			RenderCommand::copyFrameBufferData(srcID, dstID, RenderCommand::BufferBit::DEPTH_BUFFER_BIT);
			glPopDebugGroup();
		}

		if (Engine::get()->getConfig().renderConfig.renderForwardPass)
		{
			glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Forward Renderer pass");
			m_forwardRenderer->renderScene(this);
			glPopDebugGroup();
		}

		if (Engine::get()->getConfig().renderConfig.renderCustomShadersPass)
		{
			glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Custom shader render pass");
			m_forwardRenderer->renderSceneUsingCustomShader(this);
			glPopDebugGroup();
		}

		if (Engine::get()->getConfig().renderConfig.renderFoliagePass)
		{
			glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Foliage render pass");

			graphics->renderView->bind();

			for (auto&& [entity, foliage, transform] : m_registry->get().view<FoliageComponent, Transformation>().each())
			{
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				Engine::get()->getSubSystem<FoliageSystem>()->setView(primaryCameraTransform.getWorldPosition(), primaryCameraTransform.getForward());
				Engine::get()->getSubSystem<FoliageSystem>()->setFrustum(frustum);
				Engine::get()->getSubSystem<FoliageSystem>()->drawFoliage(foliage);
				glDisable(GL_BLEND);
			}

			glPopDebugGroup();
		}

		if (Engine::get()->getConfig().renderConfig.renderTerrainPass)
		{
			glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Terrain render pass");

			// Render terrain
			for (auto&& [entity, terrain, transform] : m_registry->get().view<Terrain, Transformation>().each())
			{
				m_terrainShader->use();
				m_terrainShader->setUniformValue("view", *graphics->view);
				m_terrainShader->setUniformValue("projection", *graphics->projection);
				m_terrainShader->setUniformValue("scale", terrain.getScale());
				m_terrainShader->setUniformValue("model", transform.getWorldTransformation());
				m_terrainShader->setUniformValue("width", terrain.getWidth());
				m_terrainShader->setUniformValue("height", terrain.getHeight());
				m_terrainShader->setUniformValue("lightSpaceMatrix", graphics->lightSpaceMatrix);
				m_terrainShader->setTextureInShader(graphics->shadowMap, "shadowMap", 5);
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

				auto& terrainMesh = terrain.getMesh();
				if (!terrainMesh.isEmpty())
				{
					auto vao = terrainMesh->getPrimaryMesh()->getVAO();
					RenderCommand::drawPatches(vao);
				}

			}

			glPopDebugGroup();
		}

		if (Engine::get()->getConfig().renderConfig.renderSkyboxPass)
		{
			glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Skybox render pass");
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

				if (skybox.cubemap.isEmpty()) continue;

				skybox.cubemap.get()->bind();
				skybox.cubemap.get()->setSlot(0);

				auto vao = graphics->mesh->getVAO();
				RenderCommand::draw(vao);
			}
			glDepthMask(GL_TRUE);
			glDepthFunc(GL_LESS);

			glPopDebugGroup();
		}

		

		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		//glEnable(GL_POLYGON_OFFSET_LINE);
		//glPolygonOffset(-1.0, -1.0);

		

		if (Engine::get()->getConfig().renderConfig.renderVolumetricsPass)
		{
			glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Volumetrics render pass");

			// Render Volumetrics
			for (auto&& [entity, volume, shader] : m_registry->get().view<VolumeComponent, ShaderComponent>().each())
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

			glPopDebugGroup();
		}

		// Highlight selected object
		auto objectPicker = Engine::get()->getSubSystem<ObjectPicker>();
		int selectedObject = objectPicker->getSelectedObject();
		if (selectedObject > -1)
		{
			// get entity from ID
			// 1st pass - draw mesh to depth buffer
			// -- get mesh
			// -- bind very simple shader
			// -- bind highlight FBO
			// -- draw
			// 2nd pass - use depth buffer values to apply gradient V&H
			// -- swap highlight FBO buffers
			// -- bind edge detection shader
			// -- set texture as uniform
			// -- bind highlight FBO
			// -- draw
			// 3rd pass - dilate and merge with original image
			// -- bind dilate and merge shader
			// -- set texture as uniform
			// -- set main texture as uniform
			// -- bind scene FBO
			// -- draw
			for (auto&& [entity, obj] : getRegistry().get().view<ObjectComponent>().each())
			{
				if ((entity_id)entity == selectedObject)
				{
					Entity e(entity, &getRegistry());
					auto mesh = e.tryGetComponent<MeshComponent>();

					if (!mesh) break;

					glDisable(GL_DEPTH_TEST);

					{
						glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Highlight Pass 1 - Mask");

						// 1st pass
						m_highlightRenderView->bind();
						RenderCommand::clear();
						m_highlightMaskShader->use();
						m_highlightMaskShader->setModelMatrix(e.getComponent<Transformation>().getWorldTransformation());
						m_highlightMaskShader->setViewMatrix(*graphics->view);
						m_highlightMaskShader->setProjectionMatrix(*graphics->projection);

						for (auto& m : mesh->mesh->getMeshes())
							RenderCommand::draw(m->getVAO());

						glPopDebugGroup();
					}

					{
						glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Highlight Pass 2 - Edge Detection");

						// 2nd pass
						auto& binaryMaskTexture = m_highlightRenderView->getRenderTargetTexture();
						m_highlightRenderView->swapToAdditionalTarget();
						m_highlightRenderView->bind();
						//RenderCommand::clear();
						m_highlightEdgeDetectionShader->use();
						auto width = Engine::get()->getWindow()->getWidth();
						auto height = Engine::get()->getWindow()->getHeight();
						glm::vec2 texelSize = glm::vec2(1.0 / width, 1.0 / height);
						m_highlightEdgeDetectionShader->setUniformValue("uTexelSize", texelSize);
						m_highlightEdgeDetectionShader->setTextureInShader(binaryMaskTexture, "uMaskTex", 1);

						auto vao = m_quadUI.getComponent<MeshComponent>().mesh.get()->getPrimaryMesh()->getVAO();
						RenderCommand::draw(vao);

						glPopDebugGroup();
					}

					{
						glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Highlight Pass 3 - Merge");

						// 3rd pass
						Resource<Texture> mainSceneRenderTargetTexture = graphics->renderView->getRenderTargetTexture();
						m_highlightRenderView->swapBackToMainTarget(); // todo optimize (i should fetch the secondary texture instead)
						auto& edgeDetectedTexture = m_highlightRenderView->getRenderTargetTexture(); // todo fix
						auto width = Engine::get()->getWindow()->getWidth();
						auto height = Engine::get()->getWindow()->getHeight();
						glm::vec2 texelSize = glm::vec2(1.0 / width, 1.0 / height);
						m_highlightMergeShader->use();
						m_highlightMergeShader->setUniformValue("uTexelSize", texelSize);
						m_highlightMergeShader->setUniformValue("uHighlightColor", glm::vec3(1.0, 0.55, 0.0));
						m_highlightMergeShader->setUniformValue("uDilationRadius", 3);
						m_highlightMergeShader->setTextureInShader(mainSceneRenderTargetTexture, "MainTexture", 0);
						m_highlightMergeShader->setTextureInShader(edgeDetectedTexture, "uEdgeTex", 1);

						graphics->renderView->bind();

						auto vao = m_quadUI.getComponent<MeshComponent>().mesh.get()->getPrimaryMesh()->getVAO();
						RenderCommand::draw(vao);

						glPopDebugGroup();
					}

					glEnable(GL_DEPTH_TEST);
				}
			}
		}

		// Render WireframeGrid
		if (Engine::get()->getConfig().renderConfig.renderWireframeGrid)
		{
			glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Wireframe Grid");

			m_wireframeGrid->shader->use();
			m_wireframeGrid->shader->setModelMatrix(glm::mat4(1.0));
			m_wireframeGrid->shader->setViewMatrix(*graphics->view);
			m_wireframeGrid->shader->setProjectionMatrix(*graphics->projection);
			m_wireframeGrid->shader->setUniformValue("color", glm::vec3(0.6, 0.6, 0.6));

			m_wireframeGrid->vao->Bind();

			glLineWidth(2); // Size in pixels
			glDrawArrays(GL_LINES, 0, m_wireframeGrid->vao->GetVerticesCount());

			glPopDebugGroup();
		}

		if (m_isSimulationActive)
		{
			Engine::get()->getPhysicsSystem()->visualizePhysicsShapeDebug(this);
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

		glPopDebugGroup();
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
	



	//for (const auto& cb : m_renderCallbacks[RenderPhase::POST_RENDER_BEGIN])
	//{
	//	cb();
	//}

	//for (const auto& cb : m_renderCallbacks[RenderPhase::POST_RENDER_END])
	//{
	//	cb();
	//}
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

glm::mat4 Scene::getGameCameraView() const
{
        auto& primaryCamera = getGameCamera().getComponent<CameraComponent>();
        auto& primaryCameraTransform = getGameCamera().getComponent<Transformation>();

        return glm::lookAt(primaryCameraTransform.getWorldPosition(), primaryCameraTransform.getWorldPosition() + primaryCamera.front, primaryCamera.up);
}

void Scene::onWindowResize(int w, int h)
{
	for (auto& [name, view] : m_renderViews)
	{
		view->resize(w, h);
	}
	if (m_highlightRenderView)
	{
		m_highlightRenderView->resize(w, h);
	}

	if (m_deferredRenderer)
	{
		m_deferredRenderer->resize(w, h);
	}

    m_defaultPerspectiveProjection = glm::perspective(45.0f, (float)w / h, 0.1f, 1000.0f);
    m_defaultUIProjection = glm::ortho(0.0f, (float)w, (float)h, 0.0f, -1.0f, 1.0f);

    auto& reg = m_registry->getRegistry();
    auto view = reg.view<CameraComponent>();
	for (auto entity : view)
	{
		view.get<CameraComponent>(entity).aspect = (float)w / h;
	}

	if (m_postProcessProjector)
	{
		m_postProcessProjector->init(w, h);
	}
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

		try
		{
			nsc.script->onCreate();
		}
		catch (const std::exception& e)
		{
			logError("Native Script Error occured: {}", e.what());
		}
		

		//nsc.script->eventHandler = Engine::get()->getEventSystem()->bindToLayer(gameEventLayer->name);
	}

	gameEventLayer->setEnabled(true);

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
		try
		{
			nsc.script->onDestroy();
		}
		catch (const std::exception& e)
		{
			logError("Native Script Error occured: ", e.what());
		}
		//gameEventLayer->unsubscribe(nsc.script); // TODO fix, ican simply remove the layer instead of all the scripts from the layer
	}

	getRegistry().getRegistry().clear();

	Archiver::deserializeScene(m_serializedScene, *this);

	gameEventLayer->setEnabled(false);

	m_isSimulationActive = false;
}

physx::PxScene* Scene::getPhysicsScene() const
{
	return m_PhysicsScene;
}

void Scene::addRenderView(const std::string& name, int x, int y, int w, int h, const Entity& e)
{
	m_renderViews[name] = std::make_shared<RenderView>(Viewport{x, y, w, h}, e, name);
}

unsigned int Scene::getRenderViewFrameBufferID(const std::string& name) const
{
	auto renderView = getRenderView(name);

	if (!renderView)
	{
		return 0;
	}

	return renderView->getRenderTargetFrameBufferID();
}

unsigned int Scene::getRenderViewTextureID(const std::string& name) const
{
	auto renderView = getRenderView(name);

	if (!renderView)
	{
		return 0;
	}

	return renderView->getRenderTargetTextureID();
}

unsigned int Scene::getGameRenderViewTextureID() const
{
	return getRenderViewTextureID("Game View"); // todo fix
}

unsigned int Scene::getGameRenderViewFrameBufferID() const
{
	return getRenderViewFrameBufferID("Game View"); // todo fix
}

std::shared_ptr<RenderView> Scene::getRenderView(const std::string& name) const
{
	auto iter = m_renderViews.find(name);
	if (iter == m_renderViews.end())
	{
		logWarning("Could not find render view {}", name);
		return nullptr;
	}

	return iter->second;
}

bool Scene::isSimulationActive() const
{
	return m_isSimulationActive;
}