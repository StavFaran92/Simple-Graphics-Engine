#include "runtime/Scene.h"

#include <filesystem>
#include <fstream>
#include <cereal/archives/json.hpp>

#include "core/Engine.h"
#include "core/CoroutineSystem.h"
#include "core/Logger.h"
#include "runtime/Context.h"
#include "core/Window.h"
#include "systems/ObjectPicker.h"
#include "render/Shader.h"
#include "runtime/Entity.h"
#include "component/Transformation.h"
#include "geometry/Mesh.h"
#include "component/RenderableComponent.h"
#include "component/WaterBodyComponent.h"
#include "render/Material.h"
#include "component/ScriptableEntity.h"
#include "physics/PhysicsSystem.h"
#include "animation/AnimationSystem.h"
#include "systems/ShadowSystem.h"
#include "lights/LightSystem.h"
#include "systems/TimeManager.h"
#include "render/UniformBufferObject.h"
#include "render/DeferredRenderer.h"
#include "render/Renderer.h"
#include "geometry/ShapeFactory.h"
#include <GL/glew.h>

#include "render/RenderCommand.h"
#include "render/IBL.h"
#include "core/Registry.h"
#include "serialize/Archiver.h"
#include "animation/Animator.h"
#include "component/Terrain.h"
#include "geometry/Frustum.h"
#include "geometry/Model.h"
#include "render/Graphics.h"
#include "utils/DebugHelper.h"
#include "render/RenderView.h"
#include "core/EventSystem.h"
#include "core/EngineConfig.h"
#include "geometry/WireframeGrid.h"

#include "systems/FoliageSystem.h"
#include "systems/WaterSystem.h"
#include "systems/VolumetricCloudsSystem.h"
#include "component/CameraComponent.h"
#include "component/MeshRendererComponent.h"
#include "component/ObjectComponent.h"
#include "component/SkyboxComponent.h"
#include "component/NativeScriptComponent.h"
#include "component/ImageComponent.h"
#include "component/PostProcessComponent.h"
#include "component/PlayerControllerComponent.h"
#include "component/VolumeComponent.h"
#include "component/VolumetricCloudsComponent.h"
#include "scripts/ScriptSystem.h"
#include "memory/BuiltInAssets.h"
#include "memory/BuiltInResources.h"
#include "systems/VolumetricSystem.h"
#include "core/Factory.h"
#include "memory/BuiltInAssets.h"
#include "core/Trace.h"
#include "core/GameEventSystem.h"

SceneResourceRef Scene::load(const std::string& fileLocation, SceneLoadDescriptor desc/* = {}*/)
{
	SceneResourceRef scene = Factory<Scene>::create();
	scene->init(Engine::get()->getContext(), scene.getUID());

	std::string filepath = desc.sourcePath;
	std::ifstream is(filepath);


	try
	{
		cereal::JSONInputArchive iarchive(is);
		SceneData sceneData;
		iarchive(sceneData);
		Archiver::deserializeScene(sceneData.m_serializedScene, scene);
	}
	catch (const cereal::Exception& e)
	{
		logError("Deserialization Error occured: {}", e.what());
	}

	// Resolve
	for (auto& cbWrapper : ComponentSerdes::getRegistry())
	{
		scene->getRegistry().get().each([&](entt::entity e) {
			Entity entity(e, &scene->getRegistry());
			cbWrapper.resolve(entity, scene);
		});
		
	}

	// Post Load
	for (auto& cbWrapper : ComponentSerdes::getRegistry())
	{
		scene->getRegistry().get().each([&](entt::entity e) {
			Entity entity(e, &scene->getRegistry());
			cbWrapper.postLoad(entity, scene);
		});
	}

	return scene;
}

SceneResourceRef Scene::create()
{
	auto scene = Factory<Scene>::create();
	scene->init(Engine::get()->getContext(), scene.getUID());
	return scene;
}

void Scene::onActivate()
{
	preloadSceneResources();
}

void Scene::onDeactivate()
{
	m_cachedResources.clear();
}

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

	for (auto& mesh : e.tryGetComponent<MeshRendererComponent>()->mesh.resource()->getMeshes())
	{
		graphics->entity = e;
		graphics->shader = m_tempOutlineShader;
		graphics->mesh = mesh.get();
		graphics->model = e.getComponent<Transformation>().getWorldTransformation();

		Engine::get()->getDeferredRenderer().render();
	}
}

void Scene::setIBLData(TextureResourceRef irradianceMap, TextureResourceRef prefilterEnvMap)
{
	m_irradianceMap = irradianceMap;
	m_prefilterEnvMap = prefilterEnvMap;
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

void Scene::init(Context* context, ResourceID rid)
{
	m_context = context;
	m_rid = rid;

	//static auto onChangedCB = [this](UUID) {
	//	makeDirty();
	//};

	m_registry = std::make_shared<SGE_Regsitry>();
	m_registry->registerOnComponentAdded([rid](Component& c) {
		
		// The following will be called for each added component
		SceneResourceRef scene = Engine::get()->getResourceManager()->getResource(rid).as<Scene>();
		//c.registerSceneDependency(scene);
		//onChangedCB(EMPTY_UUID); //for now use empty uid as im not sure it will be needed

		if (scene->isReady())
		{
			c.resolve(scene);
			c.onInit(scene);
		}
		else
		{
			// TODO should handle the case where I add a component and the scene is not yet ready so the component might enter invalid state
			logWarning("Component entered scene without proper init, this might cause issues..");

		}
	});

	m_registry->registerOnComponentRemoved([]() {
		//onChangedCB(EMPTY_UUID);
		
	});

	auto width = Engine::get()->getWindow()->getWidth();
	auto height = Engine::get()->getWindow()->getHeight();

	gameEventLayer = Engine::get()->getEventSystem()->getLayer("GameLayer");

	m_coroutineManager = std::make_shared<CoroutineSystem>();

	m_PhysicsScene = Engine::get()->getPhysicsSystem()->createScene();

	m_shadowSystem = std::make_shared<ShadowSystem>();
	if (!m_shadowSystem->init())
	{
		logError("Shadow System init failed!");
	}


	m_lightSystem = std::make_shared<LightSystem>(m_context, this);
	if (!m_lightSystem->init())
	{
		logError("Light System init failed!");
	}

	m_defaultPerspectiveProjection = glm::perspective(45.0f, (float)width / height, 0.1f, 1000.0f);

	m_defaultUIProjection = glm::ortho(0.0f, (float)Engine::get()->getWindow()->getWidth(), (float)Engine::get()->getWindow()->getHeight(), 0.0f, -1.0f, 1.0f);

	m_UIShader = Shader::load(SGE_ROOT_DIR "Resources/Engine/Shaders/UIShader.glsl");
	m_tempOutlineShader = Shader::load(SGE_ROOT_DIR "Resources/Engine/Shaders/OutlineShader.glsl");

	m_uboTime = std::make_shared<UniformBufferObject>(sizeof(float));
	m_uboTime->attachToBindPoint(0);

	// Create irradiance map using created cubemap
	TextureData defaultCubemapData;
	defaultCubemapData.target = TextureTarget::TEXTURE_CUBE_MAP;
	defaultCubemapData.width = 1;
	defaultCubemapData.height = 1;
	defaultCubemapData.channels = 3;
	defaultCubemapData.internalFormat = TextureInternalFormat::RGB;
	defaultCubemapData.format = TextureFormat::RGB;
	defaultCubemapData.type = TextureType::UNSIGNED_BYTE;
	defaultCubemapData.filter = TextureFilter::Linear;
	defaultCubemapData.wrap = TextureWrap::Clamp;
	defaultCubemapData.genMipMap = false;
	static unsigned char FULL_WHITE[3] = { 255, 255, 255 };
	for (int i = 0; i < 6; i++)
	{
		defaultCubemapData.facesData[i] = ImageBuffer(FULL_WHITE, 3);
	}
	m_irradianceMap = Texture::createTexture(defaultCubemapData);

	// Create prefilter env map using created cubemap
	TextureData defaultCubemapData2;
	defaultCubemapData2.target = TextureTarget::TEXTURE_CUBE_MAP;
	defaultCubemapData2.width = 1;
	defaultCubemapData2.height = 1;
	defaultCubemapData2.channels = 3;
	defaultCubemapData2.internalFormat = TextureInternalFormat::RGB;
	defaultCubemapData2.format = TextureFormat::RGB;
	defaultCubemapData2.type = TextureType::UNSIGNED_BYTE;
	defaultCubemapData2.filter = TextureFilter::Linear;
	defaultCubemapData2.wrap = TextureWrap::Clamp;
	defaultCubemapData2.genMipMap = false;
	for (int i = 0; i < 6; i++)
	{
		defaultCubemapData2.facesData[i] = ImageBuffer(FULL_WHITE, 3);
	}
	m_prefilterEnvMap = Texture::createTexture(defaultCubemapData2);

	// Create BRDF look up texture
	m_BRDFIntegrationLUT = IBL::generateBRDFIntegrationLUT();

	m_skyboxShader = Shader::load(SGE_ROOT_DIR "Resources/Engine/Shaders/SkyboxShader.glsl");

	addRenderView("Game View", 0, 0, Engine::get()->getWindow()->getWidth(), Engine::get()->getWindow()->getHeight(), Entity::EmptyEntity);

	m_registry->getRegistry().on_construct<ScriptableEntity>().connect<&Scene::bindScriptToLayer>(this);

	m_highlightRenderView = std::make_shared<RenderView>(Viewport{ 0, 0, Engine::get()->getWindow()->getWidth(), Engine::get()->getWindow()->getHeight() }, Entity::EmptyEntity);

	m_highlightMaskShader = Shader::load(SGE_ROOT_DIR "Resources/Engine/Shaders/HighlighMaskShader.glsl");

	ShaderLoadDescriptor loadDesc;
	loadDesc.shaderOverride = ShaderOverride::PostProcess;
	m_highlightEdgeDetectionShader = Shader::load(SGE_ROOT_DIR "Resources/Engine/Shaders/HighlightEdgeDetectionShader.glsl", loadDesc);
	m_highlightMergeShader = Shader::load(SGE_ROOT_DIR "Resources/Engine/Shaders/HighlightMergeShader.glsl", loadDesc);

	m_debugVisualizeShader = Shader::load(SGE_ROOT_DIR "Resources/Engine/Shaders/UnlitShader.glsl");

	m_wireframeGrid = std::make_shared<WireframeGrid>();

	// Scene is loaded and ready to be used. should come last.
	logInfo("Scene {} is loaded and ready.", rid);
	m_isReady = true;

	//std::vector<GLuint> data = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 }; // sum = 45

	//glGenBuffers(1, &ssbo);
	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
	//glBufferData(GL_SHADER_STORAGE_BUFFER, data.size() * sizeof(GLuint), data.data(), GL_DYNAMIC_DRAW);
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo); // Binding = 0


	//m_sampleComputeShader = Shader::create(SGE_ROOT_DIR "Resources/Engine/Shaders/SampleComputeShader.glsl");

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

void Scene::makeDirty()
{
	m_isDirty = true;
}

bool Scene::isSerializationDirty() const
{
	return m_isDirty;
}

bool Scene::isReady() const
{
	return m_isReady;
}

void Scene::update(float deltaTime)
{
	preloadSceneResources();

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

		// Run all scripts updates
		auto scriptSystem = Engine::get()->getSubSystem<ScriptSystem>();
		try
		{
			scriptSystem->callUpdate(deltaTime);
		}
		catch (const std::exception& e)
		{
			logError("Script Error occured: {}", e.what());
		}

		// Physics
		Engine::get()->getPhysicsSystem()->update(this, deltaTime);

		Engine::get()->getSubSystem<AnimationSystem>()->update(this, deltaTime);
	}

	//if (m_isDirty)
	//{
	//	preloadSceneResources();
	//	m_isDirty = false;
	//}
}

void Scene::draw(float deltaTime)
{
	auto graphics = Engine::get()->getSubSystem<Graphics>();

	for (auto& [rName, renderView] : m_renderViews)
	{
		if (!renderView->isEnabled())
			continue;

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
		graphics->view = glm::lookAt(primaryCameraTransform.getWorldPosition(), primaryCameraTransform.getWorldPosition() + primaryCamera.front, primaryCamera.up);
		graphics->projection = primaryCamera.getProjection();
		graphics->cameraPos = primaryCameraTransform.getWorldPosition();
		graphics->irradianceMap = m_irradianceMap;
		graphics->prefilterEnvMap = m_prefilterEnvMap;
		graphics->brdfLUT = m_BRDFIntegrationLUT;
		graphics->renderView = renderView;

		Frustum frustum(primaryCameraTransform.getWorldPosition(),
			primaryCamera.front,
			primaryCamera.up,
			primaryCamera.right,
			primaryCamera.aspect,
			primaryCamera.getFOVYInRadians(),
			primaryCamera.znear,
			primaryCamera.zfar);

		graphics->frustum = &frustum;

		if(Engine::get()->getConfig().renderConfig.renderShadowMap)
		{
			glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Generate Shadow Map");
			m_shadowSystem->renderToDepthMap();
			glPopDebugGroup();
		}

		graphics->lightSpaceMatrix = m_shadowSystem->getLightSpaceMat();
		graphics->shadowMap = m_shadowSystem->getShadowMap();

		for (auto&& [entity, waterBody, transform] : m_registry->get().view<WaterBodyComponent, Transformation>().each())
		{
			Engine::get()->getSubSystem<WaterSystem>()->prepareWaterBodyForRender(waterBody);
		}

		for (auto&& [entity, clouds, transform] : m_registry->get().view<VolumetricCloudsComponent, Transformation>().each())
		{
			Engine::get()->getSubSystem <VolumetricCloudsSystem>()->prepareVolumetricCloudsForRender(clouds);
		}

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

		RenderCommand::setViewport(viewport.x, viewport.y, viewport.w, viewport.h);

		glEnable(GL_DEPTH_TEST);
		graphics->renderView->bind();

		if (Engine::get()->getConfig().renderConfig.renderDeferredPass)
		{
			glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Deferred Renderer pass");
			Engine::get()->getDeferredRenderer().renderScene(this);
			unsigned int srcID = Engine::get()->getDeferredRenderer().getGBuffer().getID();
			unsigned int dstID = graphics->renderView->getRenderTargetFrameBufferID();
			RenderCommand::copyFrameBufferData(srcID, dstID, RenderCommand::BufferBit::DEPTH_BUFFER_BIT);
			glPopDebugGroup();
		}

		if (Engine::get()->getConfig().renderConfig.renderForwardPass)
		{
			glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Forward Renderer pass");
			Engine::get()->getForwardRenderer().renderScene(this);
			glPopDebugGroup();
		}

		if (Engine::get()->getConfig().renderConfig.renderCustomShadersPass)
		{
			glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Custom shader render pass");
			Engine::get()->getForwardRenderer().renderSceneUsingCustomShader(this);
			glPopDebugGroup();
		}

		if (Engine::get()->getConfig().renderConfig.renderFoliagePass)
		{
			glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Foliage render pass");

			graphics->renderView->bind();

			for (auto&& [entity, terrain, transform] : m_registry->get().view<Terrain, Transformation>().each())
			{
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				Engine::get()->getSubSystem<FoliageSystem>()->setView(primaryCameraTransform.getWorldPosition(), primaryCameraTransform.getForward());
				Engine::get()->getSubSystem<FoliageSystem>()->setFrustum(frustum);
				Engine::get()->getSubSystem<FoliageSystem>()->drawFoliage(terrain);
				glDisable(GL_BLEND);
			}

			glPopDebugGroup();
		}

		if (Engine::get()->getConfig().renderConfig.renderWaterBodyPass)
		{
			//glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Water Body render pass");

			//graphics->renderView->bind();

			

			//glPopDebugGroup();
		}

		if (Engine::get()->getConfig().renderConfig.renderTerrainPass)
		{
			glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Terrain render pass");

			ShaderResourceRef terrainShader = BuiltInResources::get<Shader>(SGE_RESOURCE_SHADER_TERRAIN);;
			terrainShader->use();

			// Render terrain
			for (auto&& [entity, terrain, transform] : m_registry->get().view<Terrain, Transformation>().each())
			{
				// TODO change bhaviour-> use default terrain material
				if (terrain.m_material.isEmpty())
					continue;

				TextureResourceRef heightmap = terrain.getHeightmap();

				if (heightmap.isEmpty())
					continue;

				terrainShader->setUniformValue("view", graphics->view);
				terrainShader->setUniformValue("projection", graphics->projection);
				terrainShader->setUniformValue("scale", terrain.getScale());
				terrainShader->setUniformValue("model", transform.getWorldTransformation());
				terrainShader->setUniformValue("width", terrain.getWidth());
				terrainShader->setUniformValue("height", terrain.getHeight());
				terrainShader->setUniformValue("lightSpaceMatrix", graphics->lightSpaceMatrix);
				terrainShader->setUniformValue("cameraPos", graphics->cameraPos);
				terrainShader->setUniformValue("layerCount", terrain.getLayerCount());
				terrainShader->bindUniformBlockToBindPoint("Time", 0);
				terrainShader->bindUniformBlockToBindPoint("Lights", 1);
				terrainShader->setTextureInShader(graphics->irradianceMap, "gIrradianceMap", 0);
				terrainShader->setTextureInShader(graphics->prefilterEnvMap, "gPrefilterEnvMap", 1);
				terrainShader->setTextureInShader(graphics->brdfLUT, "gBRDFIntegrationLUT", 2);
				terrainShader->setTextureInShader(graphics->shadowMap, "gShadowMap", 3);
				terrainShader->setTextureInShader(heightmap, "heightMap", 4);

				auto tileNoise = BuiltInAssets::getByName<TextureAsset>("SGE_TEXTURE_TILE_NOISE").resource();
				terrainShader->setTextureInShader(tileNoise, "tileNoise", 5);

				for (int i = 0; i < terrain.getLayerCount(); i++)
				{
					auto& mask = terrain.getLayer(i).mask.resource();
					auto name = "terrainLayerMask[" + std::to_string(i) + "]";
					terrainShader->setTextureInShader(mask, name, 6 + i);
				}

				terrain.m_material.resource()->use();

				//int textureCount = terrain.getTextureCount();
				//m_terrainShader->setUniformValue("textureCount", textureCount);

				//for (int i = 0; i < textureCount; i++)
				//{
				//	auto texture = terrain.getTexture(i).resource();
				//	texture.get()->setSlot(i + 1);
				//	texture.get()->bind();
				//	m_terrainShader->setUniformValue("texture_" + std::to_string(i), i + 1);

				//	auto textureBlend = terrain.getTextureBlend(i);
				//	m_terrainShader->setUniformValue("textureBlend[" + std::to_string(i) + "]", textureBlend);

				//	glm::vec2 textureScale = terrain.getTextureScale(i);
				//	m_terrainShader->setUniformValue("textureScale[" + std::to_string(i) + "]", textureScale);
				//}

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

			m_skyboxShader->setViewMatrix(graphics->view);
			m_skyboxShader->setProjectionMatrix(graphics->projection);

			for (auto&& [entity, skybox, transform] :
				m_registry->get().view<SkyboxComponent, Transformation>().each())
			{
				Entity entityhandler{ entity, m_registry.get() };
				graphics->entity = entityhandler;
				graphics->mesh = BuiltInAssets::getByName<ModelAsset>(SGE_MESH_BOX).resource()->getPrimaryMesh().get();
				graphics->model = transform.getWorldTransformation();

				if (skybox.m_cubemap.resource().isEmpty()) continue;

				skybox.m_cubemap.resource().get()->bind();
				skybox.m_cubemap.resource().get()->setSlot(0);

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
			for (auto&& [entity, volume] : m_registry->get().view<VolumeComponent>().each())
			{
				Entity entityHandler(entity, &getRegistry());
				auto& transform = entityHandler.getComponent<Transformation>();
				glm::mat4 modelTransform = transform.getWorldTransformation();
				VolumetricSystem::get()->drawVolumetric(volume, modelTransform);
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
					ModelResourceRef mesh;
					auto meshRenderer = e.tryGetComponent<MeshRendererComponent>();
					if (meshRenderer)
					{
						mesh = meshRenderer->mesh.resource();
					}
					else
					{
						auto terrain = e.tryGetComponent<Terrain>();
						if (terrain)
						{
							mesh = terrain->getMesh();
						}
					}
					if (mesh.isEmpty())
					{
						continue;
					}

					glDisable(GL_DEPTH_TEST);

					{
						glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Highlight Pass 1 - Mask");

						// 1st pass
						m_highlightRenderView->bind();
						RenderCommand::clear();
                        m_highlightMaskShader->use();

                        m_highlightMaskShader->setViewMatrix(graphics->view);
                        m_highlightMaskShader->setProjectionMatrix(graphics->projection);

                        m_highlightMaskShader->setUniformValue("isGpuInstanced", false);

                        auto animator = e.tryGetComponent<Animator>();
                        if (!animator || !animator->hasActiveAnimation())
                        {
							m_highlightMaskShader->setUniformValue("isAnimated", false);
                        }
                        else
                        {
                            std::vector<glm::mat4> finalBoneMatrices;
                            animator->getFinalBoneMatrices(mesh.get(), finalBoneMatrices);
                            for (int i = 0; i < finalBoneMatrices.size(); ++i)
                            {
                                m_highlightMaskShader->setUniformValue("finalBonesMatrices[" + std::to_string(i) + "]", finalBoneMatrices[i]);
                            }

                            m_highlightMaskShader->setUniformValue("isAnimated", true);
                        }

                        for (auto& m : mesh->getMeshes())
                        {
                            m_highlightMaskShader->setModelMatrix(e.getComponent<Transformation>().getWorldTransformation() * m->getRestTransform());
                            RenderCommand::draw(m->getVAO());
						}

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

						auto vao = BuiltInAssets::getByName<ModelAsset>(SGE_MESH_QUAD).resource()->getPrimaryMesh()->getVAO();
						RenderCommand::draw(vao);

						glPopDebugGroup();
					}

					{
						glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Highlight Pass 3 - Merge");

						// 3rd pass
						TextureResourceRef mainSceneRenderTargetTexture = graphics->renderView->getRenderTargetTexture();
						m_highlightRenderView->swapBackToMainTargetWithCopy(); // todo optimize (i should fetch the secondary texture instead)
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

						auto vao = BuiltInAssets::getByName<ModelAsset>(SGE_MESH_QUAD).resource()->getPrimaryMesh()->getVAO();
						RenderCommand::draw(vao);

						glPopDebugGroup();
					}

					glEnable(GL_DEPTH_TEST);
				}
			}
		}

		// Render physics collider
		for (auto&& [entity, transform, physics] : getRegistry().get().view<Transformation, PhysicsComponent>().each())
		{
			if ((entity_id)entity == selectedObject)
			{
				glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Physics Debug");

				m_debugVisualizeShader->use();

				m_debugVisualizeShader->setViewMatrix(graphics->view);
				m_debugVisualizeShader->setProjectionMatrix(graphics->projection);

				if(physics.isActive)
					m_debugVisualizeShader->setUniformValue("color", glm::vec3(0, 1, 0));
				else 
					m_debugVisualizeShader->setUniformValue("color", glm::vec3(.8, .8, .8));

				glDisable(GL_DEPTH_TEST);

				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				glEnable(GL_POLYGON_OFFSET_LINE);
				glPolygonOffset(-1.0, -1.0);
				glLineWidth(1); // Size in pixels

				glm::mat4 model = transform.getWorldTransformation();
				model = model * glm::translate(glm::mat4(1.0f), physics.offset);

				if (physics.shapeType == CollisionShape::BOX)
				{
					auto collisionBox = std::dynamic_pointer_cast<CollisionBox>(physics.collider);
					auto extents = collisionBox->extents;
					model = glm::scale(model, glm::vec3(extents.x*2, extents.y*2, extents.z*2));
					m_debugVisualizeShader->setModelMatrix(model);
					auto& mesh = BuiltInAssets::getByName<ModelAsset>(SGE_MESH_BOX);
					auto vao = mesh.resource()->getPrimaryMesh()->getVAO();
					RenderCommand::draw(vao);
				}

				if (physics.shapeType == CollisionShape::SPHERE)
				{
					auto collisionSphere = std::dynamic_pointer_cast<CollisionSphere>(physics.collider);
					auto radius = collisionSphere->radius;
					model = glm::scale(model, glm::vec3(radius * 2));
					m_debugVisualizeShader->setModelMatrix(model);
					auto& mesh = BuiltInAssets::getByName<ModelAsset>(SGE_MESH_SPHERE);
					auto vao = mesh.resource()->getPrimaryMesh()->getVAO();
					RenderCommand::draw(vao);
				}

				if (physics.shapeType == CollisionShape::CAPSULE)
				{
					auto collisionCapsule = std::dynamic_pointer_cast<CollisionCapsule>(physics.collider);
					float radius = collisionCapsule->radius;
					float halfHeight = collisionCapsule->halfHeight;
					glm::mat4 topSphereModel = model;
					glm::mat4 bottomSphereModel = model;
					glm::mat4 cylinderModel = model;
					cylinderModel = glm::scale(cylinderModel, glm::vec3(radius * 2, halfHeight * 2, radius * 2));
					m_debugVisualizeShader->setModelMatrix(cylinderModel);

					auto& cylinderMesh = BuiltInAssets::getByName<ModelAsset>(SGE_MESH_CYLINDER);
					auto  cylinderVao  = cylinderMesh.resource()->getPrimaryMesh()->getVAO();
					auto& sphereMesh   = BuiltInAssets::getByName<ModelAsset>(SGE_MESH_SPHERE);
					auto  sphereVao    = sphereMesh.resource()->getPrimaryMesh()->getVAO();

					// Top cap sphere
					topSphereModel = glm::translate(topSphereModel, glm::vec3(0.0f, halfHeight, 0.0f));
					topSphereModel = glm::scale(topSphereModel, glm::vec3(radius * 2));
					m_debugVisualizeShader->setModelMatrix(topSphereModel);
					RenderCommand::draw(sphereVao);

					// Bottom cap sphere
					bottomSphereModel = glm::translate(bottomSphereModel, glm::vec3(0.0f, -halfHeight, 0.0f));
					bottomSphereModel = glm::scale(bottomSphereModel, glm::vec3(radius * 2));
					m_debugVisualizeShader->setModelMatrix(bottomSphereModel);
					RenderCommand::draw(sphereVao);

					// Cylinder body
					m_debugVisualizeShader->setModelMatrix(cylinderModel);
					RenderCommand::draw(cylinderVao);
				}

				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				glDisable(GL_POLYGON_OFFSET_LINE);

				glEnable(GL_DEPTH_TEST);

				glPopDebugGroup();
			}
		}

		// Render physics collider
		for (auto&& [entity, transform, controller] : getRegistry().get().view<Transformation, PlayerController>().each())
		{
			if ((entity_id)entity == selectedObject)
			{
				glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Physics Debug");

				m_debugVisualizeShader->use();

				m_debugVisualizeShader->setViewMatrix(graphics->view);
				m_debugVisualizeShader->setProjectionMatrix(graphics->projection);
				m_debugVisualizeShader->setUniformValue("color", glm::vec3(0, 1, 0));

				glDisable(GL_DEPTH_TEST);

				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				glEnable(GL_POLYGON_OFFSET_LINE);
				glPolygonOffset(-1.0, -1.0);
				glLineWidth(1); // Size in pixels

				glm::mat4 model = transform.getWorldTransformation();
				model = model * glm::translate(glm::mat4(1.0f), controller.offset);

				float radius = controller.radius;
				float height = controller.height;
				glm::mat4 topSphereModel = model;
				glm::mat4 bottomSphereModel = model;
				glm::mat4 cylinderModel = model;
				cylinderModel = glm::scale(cylinderModel, glm::vec3(radius * 2, height, radius * 2));
				m_debugVisualizeShader->setModelMatrix(cylinderModel);

				auto& cylinderMesh = BuiltInAssets::getByName<ModelAsset>(SGE_MESH_CYLINDER);
				auto  cylinderVao = cylinderMesh.resource()->getPrimaryMesh()->getVAO();
				auto& sphereMesh = BuiltInAssets::getByName<ModelAsset>(SGE_MESH_SPHERE);
				auto  sphereVao = sphereMesh.resource()->getPrimaryMesh()->getVAO();

				// Top cap sphere
				topSphereModel = glm::translate(topSphereModel, glm::vec3(0.0f, height * .5f, 0.0f));
				topSphereModel = glm::scale(topSphereModel, glm::vec3(radius * 2));
				m_debugVisualizeShader->setModelMatrix(topSphereModel);
				RenderCommand::draw(sphereVao);

				// Bottom cap sphere
				bottomSphereModel = glm::translate(bottomSphereModel, glm::vec3(0.0f, -height * .5f, 0.0f));
				bottomSphereModel = glm::scale(bottomSphereModel, glm::vec3(radius * 2));
				m_debugVisualizeShader->setModelMatrix(bottomSphereModel);
				RenderCommand::draw(sphereVao);

				// Cylinder body
				m_debugVisualizeShader->setModelMatrix(cylinderModel);
				RenderCommand::draw(cylinderVao);


				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				glDisable(GL_POLYGON_OFFSET_LINE);

				glEnable(GL_DEPTH_TEST);

				glPopDebugGroup();
			}
		}

		// Render WireframeGrid
		if (Engine::get()->getConfig().renderConfig.renderWireframeGrid)
		{
			glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Wireframe Grid");

			m_wireframeGrid->shader->use();
			m_wireframeGrid->shader->setModelMatrix(glm::mat4(1.0));
			m_wireframeGrid->shader->setViewMatrix(graphics->view);
			m_wireframeGrid->shader->setProjectionMatrix(graphics->projection);
			m_wireframeGrid->shader->setUniformValue("color", glm::vec3(0.6, 0.6, 0.6));

			m_wireframeGrid->vao->Bind();

			glLineWidth(2); // Size in pixels
			glDrawArrays(GL_LINES, 0, m_wireframeGrid->vao->GetVerticesCount());

			glPopDebugGroup();
		}

		if (Engine::get()->getConfig().renderConfig.renderPhysicsDebugPass)
		{
			Engine::get()->getPhysicsSystem()->visualizePhysicsShapeDebug(this);
		}

		if (Engine::get()->getConfig().renderConfig.renderNonOpaquePass)
		{
			glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Non Opaque render pass");
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			Engine::get()->getForwardRenderer().renderSceneNonOpaque(this);
			glDisable(GL_BLEND);
			glPopDebugGroup();
		}

		if (Engine::get()->getConfig().renderConfig.renderDebugDataPass)
		{
			glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Debug Data render pass");
			Engine::get()->getForwardRenderer().renderDebugData(this);
			glPopDebugGroup();
		}

		{
			glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Orientation Gizmo");

			// Orientation gizmo in bottom-left corner
			glViewport(10, 10, 100, 100);  // Small 100x100 viewport at bottom-left
			glClear(GL_DEPTH_BUFFER_BIT);  // Clear depth so gizmo is always on top

			// Use orthographic or perspective projection
			glm::mat4 gizmoProjection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 10.0f);
			graphics->projection = gizmoProjection;

			// Use ONLY the rotation part of your view matrix (no translation)
			glm::mat3 rotation = glm::mat3(graphics->view);
			glm::mat4 gizmoView = glm::mat4(rotation);
			gizmoView = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, -3)) * gizmoView;
			graphics->view = gizmoView;

			// Draw three lines from origin
			DebugHelper::getInstance().drawLine(glm::vec3(0), glm::vec3(1, 0, 0), glm::vec3(1, 0, 0), 2);
			DebugHelper::getInstance().drawLine(glm::vec3(0), glm::vec3(0, 1, 0), glm::vec3(0, 1, 0), 2);
			DebugHelper::getInstance().drawLine(glm::vec3(0), glm::vec3(0, 0, 1), glm::vec3(0, 0, 1), 2);

			// Reset viewport
			RenderCommand::setViewport(viewport.x, viewport.y, viewport.w, viewport.h);

			glPopDebugGroup();
		}

		// Render UI
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		m_UIShader->use();
		m_UIShader->setProjectionMatrix(m_defaultUIProjection);
		auto vao = BuiltInAssets::getByName<ModelAsset>(SGE_MESH_QUAD).resource()->getPrimaryMesh()->getVAO();

		for (auto&& [entity, image] : m_registry->get().view<ImageComponent>().each())
		{
			Entity entityhandler{ entity, m_registry.get() };
			graphics->entity = entityhandler;
			image.image.resource()->bind();
			image.image.resource()->setSlot(0);

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

		

		
#if 0 // TODO make use of this quite usefull camera frustum debug code
		auto gameCameraTransform = getRenderView("Game View")->getCamera().getComponent<Transformation>();
		auto gameCameraComponent = getRenderView("Game View")->getCamera().getComponent<CameraComponent>();
		auto gameCameraView = glm::lookAt(gameCameraTransform.getWorldPosition(), gameCameraTransform.getWorldPosition() + gameCameraComponent.front, gameCameraComponent.up);

		glm::mat4 view = gameCameraView;
		glm::mat4 projection = graphics->projection;
		glm::mat4 invViewProj = glm::inverse(projection * view);

		// NDC cube corners
		static glm::vec3 ndcCorners[8] = {
			{-1, -1, -1}, {1, -1, -1},
			{-1,  1, -1}, {1,  1, -1},
			{-1, -1,  1}, {1, -1,  1},
			{-1,  1,  1}, {1,  1,  1}
		};

		// Transform to world space
		static glm::vec3 worldCorners[8];
		for (int i = 0; i < 8; ++i) {
			glm::vec4 p = invViewProj * glm::vec4(ndcCorners[i], 1.0f);
			worldCorners[i] = glm::vec3(p) / p.w;
		}


		auto& debug = DebugHelper::getInstance();

		// Near plane
		debug.drawLine(worldCorners[0], worldCorners[1]);
		debug.drawLine(worldCorners[1], worldCorners[3]);
		debug.drawLine(worldCorners[3], worldCorners[2]);
		debug.drawLine(worldCorners[2], worldCorners[0]);

		// Far plane
		debug.drawLine(worldCorners[4], worldCorners[5]);
		debug.drawLine(worldCorners[5], worldCorners[7]);
		debug.drawLine(worldCorners[7], worldCorners[6]);
		debug.drawLine(worldCorners[6], worldCorners[4]);

		// Connect near to far
		for (int i = 0; i < 4; ++i) {
			debug.drawLine(worldCorners[i], worldCorners[i + 4]);
		}

#endif

		if (Engine::get()->getConfig().renderConfig.renderPostProcess)
		{
			glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Post Process render pass");

			// Render Post Process Effects
			for (auto&& [entity, postProcess] : m_registry->get().view<PostProcessComponent>().each())
			{
				TextureResourceRef renderTargetTexture = graphics->renderView->getRenderTargetTexture();
				renderView->swapToAdditionalTarget();
				renderView->bind();
				RenderCommand::clear();
				glDisable(GL_DEPTH_TEST);
				// TODO assert post process shader

				// bind shader
				auto shader = postProcess.shader.resource();

				if (shader.isEmpty())
				{
					continue;
				}

				shader->use();

				// read texture from graphics FBO
				shader->setTextureInShader(renderTargetTexture, "MainTexture", 0); //todo check slot

				//shader->setModelMatrix(glm::mat4(1.0));
				//shader->setViewMatrix(graphics->view);
				//shader->setProjectionMatrix(graphics->projection);

				auto viewport = renderView->getViewport();
				shader->setUniformValue("screenSize", glm::vec2(viewport.w, viewport.h));
				glm::vec2 texelSize = glm::vec2(1.0 / viewport.w, 1.0 / viewport.h);
				shader->setUniformValue("texelSize", texelSize);

				//shader->setUniformValue("cameraPos", graphics->cameraPos);
				//shader->setUniformValue("cameraLookAt", primaryCamera.front);

				// bind mesh
				auto vao = BuiltInAssets::getByName<ModelAsset>(SGE_MESH_QUAD).resource()->getPrimaryMesh()->getVAO();

				// in frag shader i need access to mesh extentes & main texture -> set uniforms

				// draw
				RenderCommand::draw(vao);

				renderView->swapBackToMainTargetWithCopy();
				renderView->bind();
				glEnable(GL_DEPTH_TEST);
			}

			glPopDebugGroup();
		}

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
	Entity e = m_registry->createEntity();
	e.addComponent<ObjectComponent>(e, name);
	return e;

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

	Engine::get()->getDeferredRenderer().resize(w, h);

    m_defaultPerspectiveProjection = glm::perspective(45.0f, (float)w / h, 0.1f, 1000.0f);
    m_defaultUIProjection = glm::ortho(0.0f, (float)w, (float)h, 0.0f, -1.0f, 1.0f);

    auto& reg = m_registry->getRegistry();
    auto view = reg.view<CameraComponent>();
	for (auto entity : view)
	{
		view.get<CameraComponent>(entity).aspect = (float)w / h;
	}
}


void Scene::clear()
{
	m_registry->get().clear();
}

void Scene::close()
{
	clear();
}

void Scene::addCoroutine(const std::function<bool(float)>& coroutine)
{
	m_coroutineManager->addCoroutine(coroutine);
}

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

// ============================================================
//  SceneAsset (Asset wrapper)
// ============================================================

void SceneAsset::serialize(nlohmann::json& j) const
{
	// SceneAsset itself does not currently track extra metadata
	// beyond what is stored in the Scene resource and registry.
	// Keep this as an explicit empty object for future extension.
	j = nlohmann::json::object();
}

void SceneAsset::deserialize(const nlohmann::json& j)
{
	// No scene-asset specific fields to restore yet.
	(void)j;
}

void Scene::startSimulation()
{
	if (m_isSimulationActive)
	{
		logWarning("Simulation already active.");
		return;
	}

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

	// Run all scripts create
	auto scriptSystem = Engine::get()->getSubSystem<ScriptSystem>();
	for (auto&& [entity, script] : m_registry->get().view<ScriptComponent>().each())
	{
		scriptSystem->loadScript(script);
	}

	try
	{
		scriptSystem->callCreate();
	}
	catch (const std::exception& e)
	{
		logError("Script Error occured: {}", e.what());
	}

	for (auto&& [entity, animator, mesh] : m_registry->get().view<Animator, MeshRendererComponent>().each())
	{
		animator.onStart();
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

	// Run all scripts destroy
	auto scriptSystem = Engine::get()->getSubSystem<ScriptSystem>();
	try
	{
		scriptSystem->callDestroy();
	}
	catch (const std::exception& e)
	{
		logError("Script Error occured: {}", e.what());
	}

	Engine::get()->getSubSystem<GameEventSystem>()->clean();

	getRegistry().getRegistry().clear();

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

void Scene::setRenderViewEnabled(const std::string& name, bool enabled)
{
	auto renderView = getRenderView(name);

	if (renderView)
	{
		renderView->setEnabled(enabled);
	}
}

void Scene::setGameRenderViewEnabled(bool enabled)
{
	setRenderViewEnabled("Game View", enabled);
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

void Scene::preloadSceneResources()
{
	// Hack, I need THIS scene but cannot access it ATM
	SerializedScene serializedScene = Archiver::serializeScene(Engine::get()->getContext()->getActiveScene());
	for (auto& e : serializedScene.serializedEntities)
	{
		for (auto& c : e.components)
		{
			std::vector<AssetRef<Asset>> assets = c->gatherDependencies();
			for (auto& asset : assets)
			{
				if (m_cachedResources[asset.getUID()].version != asset.getVersion())
				{
					auto cachedResource = Scene::CachedResource{
						asset.resource(),
						asset.getVersion()
					};
					m_cachedResources[asset.getUID()] = cachedResource;

					Trace::updateSceneResourceVersionCache(m_rid, asset.getUID(), asset.getVersion());
				}
			}
		}
	}
}

bool Scene::isSimulationActive() const
{
	return m_isSimulationActive;
}
