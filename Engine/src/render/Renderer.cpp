//#include "render/Renderer.h"
//
//#include <GL/glew.h>
//#include <glm/glm.hpp>
//#include <glm/gtc/matrix_transform.hpp>
//
//#include "core/Engine.h"
//#include "core/Window.h"
//#include "runtime/Scene.h"
//#include "runtime/Entity.h"
//#include "render/Graphics.h"
//#include "render/RenderCommand.h"
//#include "render/RenderView.h"
//#include "render/Shader.h"
//#include "render/DeferredRenderer.h"
//#include "render/ForwardRenderer.h"
//#include "component/Transformation.h"
//#include "component/CameraComponent.h"
//#include "component/WaterBodyComponent.h"
//#include "component/VolumetricCloudsComponent.h"
//#include "component/MeshRendererComponent.h"
//#include "component/ObjectComponent.h"
//#include "component/SkyboxComponent.h"
//#include "component/VolumeComponent.h"
//#include "component/ImageComponent.h"
//#include "component/PostProcessComponent.h"
//#include "component/PlayerControllerComponent.h"
//#include "component/Terrain.h"
//#include "systems/TimeManager.h"
//#include "systems/WaterSystem.h"
//#include "systems/VolumetricCloudsSystem.h"
//#include "systems/FoliageSystem.h"
//#include "systems/VolumetricSystem.h"
//#include "systems/ObjectPicker.h"
//#include "physics/PhysicsSystem.h"
//#include "physics/Colliders.h"
//#include "animation/Animator.h"
//#include "geometry/Frustum.h"
//#include "geometry/Model.h"
//#include "geometry/Mesh.h"
//#include "geometry/WireframeGrid.h"
//#include "memory/BuiltInAssets.h"
//#include "memory/BuiltInResources.h"
//#include "utils/DebugHelper.h"
//#include "core/EngineConfig.h"
//
//void cameraCalculateOrientation(Transformation& transform, CameraComponent& cameraComponent);
//
//void Renderer::render(Scene* scene, float deltaTime)
//{
//	auto graphics = Engine::get()->getSubSystem<Graphics>();
//
//	for (auto& [rName, renderView] : m_renderViews)
//	{
//		if (!renderView->isEnabled())
//			continue;
//
//		auto viewport = renderView->getViewport();
//
//		renderView->bind();
//
//		glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, renderView->getName().c_str());
//
//		RenderCommand::clear();
//
//		RenderCommand::setViewport(viewport.x, viewport.y, viewport.w, viewport.h);
//
//		const Entity& camera = renderView->getCamera();
//
//		auto& primaryCamera = camera.getComponent<CameraComponent>();
//		auto& primaryCameraTransform = camera.getComponent<Transformation>();
//
//		cameraCalculateOrientation(primaryCameraTransform, primaryCamera);
//
//		graphics->scene = scene;
//		graphics->context = m_context;
//		graphics->view = glm::lookAt(primaryCameraTransform.getWorldPosition(), primaryCameraTransform.getWorldPosition() + primaryCamera.front, primaryCamera.up);
//		graphics->projection = primaryCamera.getProjection();
//		graphics->cameraPos = primaryCameraTransform.getWorldPosition();
//		graphics->irradianceMap = m_irradianceMap;
//		graphics->prefilterEnvMap = m_prefilterEnvMap;
//		graphics->brdfLUT = m_BRDFIntegrationLUT;
//		graphics->renderView = renderView;
//
//		Frustum frustum(primaryCameraTransform.getWorldPosition(),
//			primaryCamera.front,
//			primaryCamera.up,
//			primaryCamera.right,
//			primaryCamera.aspect,
//			primaryCamera.getFOVYInRadians(),
//			primaryCamera.znear,
//			primaryCamera.zfar);
//
//		graphics->frustum = &frustum;
//
//		if (Engine::get()->getConfig().renderConfig.renderShadowMap)
//		{
//			glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Generate Shadow Map");
//			m_shadowSystem->renderToDepthMap();
//			glPopDebugGroup();
//		}
//
//		graphics->lightSpaceMatrix = m_shadowSystem->getLightSpaceMat();
//		graphics->shadowMap = m_shadowSystem->getShadowMap();
//
//		for (auto&& [entity, waterBody, transform] : scene->getRegistry().get().view<WaterBodyComponent, Transformation>().each())
//		{
//			Engine::get()->getSubSystem<WaterSystem>()->prepareWaterBodyForRender(waterBody);
//		}
//
//		for (auto&& [entity, clouds, transform] : scene->getRegistry().get().view<VolumetricCloudsComponent, Transformation>().each())
//		{
//			Engine::get()->getSubSystem <VolumetricCloudsSystem>()->prepareVolumetricCloudsForRender(clouds);
//		}
//
//		// Set time elapsed
//		auto elapsed = (float)Engine::get()->getTimeManager()->getElapsedTime(TimeManager::Duration::MilliSeconds) / 1000;
//
//		m_uboTime->bind();
//		m_uboTime->setData(0, sizeof(float), &elapsed);
//		m_uboTime->unbind();
//
//		RenderCommand::setViewport(viewport.x, viewport.y, viewport.w, viewport.h);
//
//		glEnable(GL_DEPTH_TEST);
//		graphics->renderView->bind();
//
//		if (Engine::get()->getConfig().renderConfig.renderDeferredPass)
//		{
//			glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Deferred Renderer pass");
//			Engine::get()->getDeferredRenderer().renderScene(scene);
//			unsigned int srcID = Engine::get()->getDeferredRenderer().getGBuffer().getID();
//			unsigned int dstID = graphics->renderView->getRenderTargetFrameBufferID();
//			RenderCommand::copyFrameBufferData(srcID, dstID, RenderCommand::BufferBit::DEPTH_BUFFER_BIT);
//			glPopDebugGroup();
//		}
//
//		if (Engine::get()->getConfig().renderConfig.renderForwardPass)
//		{
//			glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Forward Renderer pass");
//			Engine::get()->getForwardRenderer().renderScene(scene);
//			glPopDebugGroup();
//		}
//
//		if (Engine::get()->getConfig().renderConfig.renderCustomShadersPass)
//		{
//			glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Custom shader render pass");
//			Engine::get()->getForwardRenderer().renderSceneUsingCustomShader(scene);
//			glPopDebugGroup();
//		}
//
//		if (Engine::get()->getConfig().renderConfig.renderFoliagePass)
//		{
//			glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Foliage render pass");
//
//			graphics->renderView->bind();
//
//			for (auto&& [entity, terrain, transform] : scene->getRegistry().get().view<Terrain, Transformation>().each())
//			{
//				glEnable(GL_BLEND);
//				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//				Engine::get()->getSubSystem<FoliageSystem>()->setView(primaryCameraTransform.getWorldPosition(), primaryCameraTransform.getForward());
//				Engine::get()->getSubSystem<FoliageSystem>()->setFrustum(frustum);
//				Engine::get()->getSubSystem<FoliageSystem>()->drawFoliage(terrain);
//				glDisable(GL_BLEND);
//			}
//
//			glPopDebugGroup();
//		}
//
//		if (Engine::get()->getConfig().renderConfig.renderWaterBodyPass)
//		{
//			//glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Water Body render pass");
//
//			//graphics->renderView->bind();
//
//
//
//			//glPopDebugGroup();
//		}
//
//		if (Engine::get()->getConfig().renderConfig.renderTerrainPass)
//		{
//			glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Terrain render pass");
//
//			ShaderResourceRef terrainShader = BuiltInResources::get<Shader>(SGE_RESOURCE_SHADER_TERRAIN);;
//			terrainShader->use();
//
//			// Render terrain
//			for (auto&& [entity, terrain, transform] : scene->getRegistry().get().view<Terrain, Transformation>().each())
//			{
//				// TODO change bhaviour-> use default terrain material
//				if (terrain.m_material.isEmpty())
//					continue;
//
//				TextureResourceRef heightmap = terrain.getHeightmap();
//
//				if (heightmap.isEmpty())
//					continue;
//
//				terrainShader->setUniformValue("view", graphics->view);
//				terrainShader->setUniformValue("projection", graphics->projection);
//				terrainShader->setUniformValue("scale", terrain.getScale());
//				terrainShader->setUniformValue("model", transform.getWorldTransformation());
//				terrainShader->setUniformValue("width", terrain.getWidth());
//				terrainShader->setUniformValue("height", terrain.getHeight());
//				terrainShader->setUniformValue("lightSpaceMatrix", graphics->lightSpaceMatrix);
//				terrainShader->setUniformValue("cameraPos", graphics->cameraPos);
//				terrainShader->setUniformValue("layerCount", terrain.getLayerCount());
//				terrainShader->bindUniformBlockToBindPoint("Time", 0);
//				terrainShader->bindUniformBlockToBindPoint("Lights", 1);
//				terrainShader->setTextureInShader(graphics->irradianceMap, "gIrradianceMap", 0);
//				terrainShader->setTextureInShader(graphics->prefilterEnvMap, "gPrefilterEnvMap", 1);
//				terrainShader->setTextureInShader(graphics->brdfLUT, "gBRDFIntegrationLUT", 2);
//				terrainShader->setTextureInShader(graphics->shadowMap, "gShadowMap", 3);
//				terrainShader->setTextureInShader(heightmap, "heightMap", 4);
//
//				auto tileNoise = BuiltInAssets::getByName<TextureAsset>("SGE_TEXTURE_TILE_NOISE").resource();
//				terrainShader->setTextureInShader(tileNoise, "tileNoise", 5);
//
//				for (int i = 0; i < terrain.getLayerCount(); i++)
//				{
//					auto& mask = terrain.getLayer(i).mask.resource();
//					auto name = "terrainLayerMask[" + std::to_string(i) + "]";
//					terrainShader->setTextureInShader(mask, name, 6 + i);
//				}
//
//				terrain.m_material.resource()->use();
//
//				//int textureCount = terrain.getTextureCount();
//				//m_terrainShader->setUniformValue("textureCount", textureCount);
//
//				//for (int i = 0; i < textureCount; i++)
//				//{
//				//	auto texture = terrain.getTexture(i).resource();
//				//	texture.get()->setSlot(i + 1);
//				//	texture.get()->bind();
//				//	m_terrainShader->setUniformValue("texture_" + std::to_string(i), i + 1);
//
//				//	auto textureBlend = terrain.getTextureBlend(i);
//				//	m_terrainShader->setUniformValue("textureBlend[" + std::to_string(i) + "]", textureBlend);
//
//				//	glm::vec2 textureScale = terrain.getTextureScale(i);
//				//	m_terrainShader->setUniformValue("textureScale[" + std::to_string(i) + "]", textureScale);
//				//}
//
//				auto& terrainMesh = terrain.getMesh();
//				if (!terrainMesh.isEmpty())
//				{
//					auto vao = terrainMesh->getPrimaryMesh()->getVAO();
//					RenderCommand::drawPatches(vao);
//				}
//
//			}
//
//			glPopDebugGroup();
//		}
//
//		if (Engine::get()->getConfig().renderConfig.renderSkyboxPass)
//		{
//			glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Skybox render pass");
//			// Render skybox
//			glDepthMask(GL_FALSE);
//			glDepthFunc(GL_LEQUAL);
//			m_skyboxShader->use();
//			renderView->bind();
//
//			m_skyboxShader->setViewMatrix(graphics->view);
//			m_skyboxShader->setProjectionMatrix(graphics->projection);
//
//			for (auto&& [entity, skybox, transform] :
//				scene->getRegistry().get().view<SkyboxComponent, Transformation>().each())
//			{
//				Entity entityhandler{ entity, m_registry.get() };
//				graphics->entity = entityhandler;
//				graphics->mesh = BuiltInAssets::getByName<ModelAsset>(SGE_MESH_BOX).resource()->getPrimaryMesh().get();
//				graphics->model = transform.getWorldTransformation();
//
//				if (skybox.m_cubemap.resource().isEmpty()) continue;
//
//				skybox.m_cubemap.resource().get()->bind();
//				skybox.m_cubemap.resource().get()->setSlot(0);
//
//				auto vao = graphics->mesh->getVAO();
//				RenderCommand::draw(vao);
//			}
//			glDepthMask(GL_TRUE);
//			glDepthFunc(GL_LESS);
//
//			glPopDebugGroup();
//		}
//
//
//
//		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
//		//glEnable(GL_POLYGON_OFFSET_LINE);
//		//glPolygonOffset(-1.0, -1.0);
//
//
//
//		if (Engine::get()->getConfig().renderConfig.renderVolumetricsPass)
//		{
//			glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Volumetrics render pass");
//
//			// Render Volumetrics
//			for (auto&& [entity, volume] : scene->getRegistry().get().view<VolumeComponent>().each())
//			{
//				Entity entityHandler(entity, &getRegistry());
//				auto& transform = entityHandler.getComponent<Transformation>();
//				glm::mat4 modelTransform = transform.getWorldTransformation();
//				VolumetricSystem::get()->drawVolumetric(volume, modelTransform);
//			}
//
//			glPopDebugGroup();
//		}
//
//		// Highlight selected object
//		auto objectPicker = Engine::get()->getSubSystem<ObjectPicker>();
//		int selectedObject = objectPicker->getSelectedObject();
//		if (selectedObject > -1)
//		{
//			// get entity from ID
//			// 1st pass - draw mesh to depth buffer
//			// -- get mesh
//			// -- bind very simple shader
//			// -- bind highlight FBO
//			// -- draw
//			// 2nd pass - use depth buffer values to apply gradient V&H
//			// -- swap highlight FBO buffers
//			// -- bind edge detection shader
//			// -- set texture as uniform
//			// -- bind highlight FBO
//			// -- draw
//			// 3rd pass - dilate and merge with original image
//			// -- bind dilate and merge shader
//			// -- set texture as uniform
//			// -- set main texture as uniform
//			// -- bind scene FBO
//			// -- draw
//			for (auto&& [entity, obj] : scene->getRegistry().get().view<ObjectComponent>().each())
//			{
//				if ((entity_id)entity == selectedObject)
//				{
//					Entity e(entity, &getRegistry());
//					ModelResourceRef mesh;
//					auto meshRenderer = e.tryGetComponent<MeshRendererComponent>();
//					if (meshRenderer)
//					{
//						mesh = meshRenderer->mesh.resource();
//					}
//					else
//					{
//						auto terrain = e.tryGetComponent<Terrain>();
//						if (terrain)
//						{
//							mesh = terrain->getMesh();
//						}
//					}
//					if (mesh.isEmpty())
//					{
//						continue;
//					}
//
//					glDisable(GL_DEPTH_TEST);
//
//					{
//						glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Highlight Pass 1 - Mask");
//
//						// 1st pass
//						m_highlightRenderView->bind();
//						RenderCommand::clear();
//						m_highlightMaskShader->use();
//
//						m_highlightMaskShader->setViewMatrix(graphics->view);
//						m_highlightMaskShader->setProjectionMatrix(graphics->projection);
//
//						m_highlightMaskShader->setUniformValue("isGpuInstanced", false);
//
//						auto animator = e.tryGetComponent<Animator>();
//						if (!animator || !animator->hasActiveAnimation())
//						{
//							m_highlightMaskShader->setUniformValue("isAnimated", false);
//						}
//						else
//						{
//							std::vector<glm::mat4> finalBoneMatrices;
//							animator->getFinalBoneMatrices(mesh, finalBoneMatrices);
//							for (int i = 0; i < finalBoneMatrices.size(); ++i)
//							{
//								m_highlightMaskShader->setUniformValue("finalBonesMatrices[" + std::to_string(i) + "]", finalBoneMatrices[i]);
//							}
//
//							m_highlightMaskShader->setUniformValue("isAnimated", true);
//						}
//
//						for (auto& m : mesh->getMeshes())
//						{
//							m_highlightMaskShader->setModelMatrix(e.getComponent<Transformation>().getWorldTransformation() * m->getRestTransform());
//							RenderCommand::draw(m->getVAO());
//						}
//
//						glPopDebugGroup();
//					}
//
//					{
//						glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Highlight Pass 2 - Edge Detection");
//
//						// 2nd pass
//						auto& binaryMaskTexture = m_highlightRenderView->getRenderTargetTexture();
//						m_highlightRenderView->swapToAdditionalTarget();
//						m_highlightRenderView->bind();
//						//RenderCommand::clear();
//						m_highlightEdgeDetectionShader->use();
//						auto width = Engine::get()->getWindow()->getWidth();
//						auto height = Engine::get()->getWindow()->getHeight();
//						glm::vec2 texelSize = glm::vec2(1.0 / width, 1.0 / height);
//						m_highlightEdgeDetectionShader->setUniformValue("uTexelSize", texelSize);
//						m_highlightEdgeDetectionShader->setTextureInShader(binaryMaskTexture, "uMaskTex", 1);
//
//						auto vao = BuiltInAssets::getByName<ModelAsset>(SGE_MESH_QUAD).resource()->getPrimaryMesh()->getVAO();
//						RenderCommand::draw(vao);
//
//						glPopDebugGroup();
//					}
//
//					{
//						glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Highlight Pass 3 - Merge");
//
//						// 3rd pass
//						TextureResourceRef mainSceneRenderTargetTexture = graphics->renderView->getRenderTargetTexture();
//						m_highlightRenderView->swapBackToMainTargetWithCopy(); // todo optimize (i should fetch the secondary texture instead)
//						auto& edgeDetectedTexture = m_highlightRenderView->getRenderTargetTexture(); // todo fix
//						auto width = Engine::get()->getWindow()->getWidth();
//						auto height = Engine::get()->getWindow()->getHeight();
//						glm::vec2 texelSize = glm::vec2(1.0 / width, 1.0 / height);
//						m_highlightMergeShader->use();
//						m_highlightMergeShader->setUniformValue("uTexelSize", texelSize);
//						m_highlightMergeShader->setUniformValue("uHighlightColor", glm::vec3(1.0, 0.55, 0.0));
//						m_highlightMergeShader->setUniformValue("uDilationRadius", 3);
//						m_highlightMergeShader->setTextureInShader(mainSceneRenderTargetTexture, "MainTexture", 0);
//						m_highlightMergeShader->setTextureInShader(edgeDetectedTexture, "uEdgeTex", 1);
//
//						graphics->renderView->bind();
//
//						auto vao = BuiltInAssets::getByName<ModelAsset>(SGE_MESH_QUAD).resource()->getPrimaryMesh()->getVAO();
//						RenderCommand::draw(vao);
//
//						glPopDebugGroup();
//					}
//
//					glEnable(GL_DEPTH_TEST);
//				}
//			}
//		}
//
//		// Render physics collider
//		for (auto&& [entity, transform, physics] : getRegistry().get().view<Transformation, PhysicsComponent>().each())
//		{
//			if ((entity_id)entity == selectedObject)
//			{
//				glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Physics Debug");
//
//				m_debugVisualizeShader->use();
//
//				m_debugVisualizeShader->setViewMatrix(graphics->view);
//				m_debugVisualizeShader->setProjectionMatrix(graphics->projection);
//
//				if (physics.isActive)
//					m_debugVisualizeShader->setUniformValue("color", glm::vec3(0, 1, 0));
//				else
//					m_debugVisualizeShader->setUniformValue("color", glm::vec3(.8, .8, .8));
//
//				glDisable(GL_DEPTH_TEST);
//
//				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
//				glEnable(GL_POLYGON_OFFSET_LINE);
//				glPolygonOffset(-1.0, -1.0);
//				glLineWidth(1); // Size in pixels
//
//				glm::mat4 model = transform.getWorldTransformation();
//				model = model * glm::translate(glm::mat4(1.0f), physics.offset);
//
//				if (physics.shapeType == CollisionShape::BOX)
//				{
//					auto collisionBox = std::dynamic_pointer_cast<CollisionBox>(physics.collider);
//					auto extents = collisionBox->extents;
//					model = glm::scale(model, glm::vec3(extents.x * 2, extents.y * 2, extents.z * 2));
//					m_debugVisualizeShader->setModelMatrix(model);
//					auto& mesh = BuiltInAssets::getByName<ModelAsset>(SGE_MESH_BOX);
//					auto vao = mesh.resource()->getPrimaryMesh()->getVAO();
//					RenderCommand::draw(vao);
//				}
//
//				if (physics.shapeType == CollisionShape::SPHERE)
//				{
//					auto collisionSphere = std::dynamic_pointer_cast<CollisionSphere>(physics.collider);
//					auto radius = collisionSphere->radius;
//					model = glm::scale(model, glm::vec3(radius * 2));
//					m_debugVisualizeShader->setModelMatrix(model);
//					auto& mesh = BuiltInAssets::getByName<ModelAsset>(SGE_MESH_SPHERE);
//					auto vao = mesh.resource()->getPrimaryMesh()->getVAO();
//					RenderCommand::draw(vao);
//				}
//
//				if (physics.shapeType == CollisionShape::CAPSULE)
//				{
//					auto collisionCapsule = std::dynamic_pointer_cast<CollisionCapsule>(physics.collider);
//					float radius = collisionCapsule->radius;
//					float halfHeight = collisionCapsule->halfHeight;
//					glm::mat4 topSphereModel = model;
//					glm::mat4 bottomSphereModel = model;
//					glm::mat4 cylinderModel = model;
//					cylinderModel = glm::scale(cylinderModel, glm::vec3(radius * 2, halfHeight * 2, radius * 2));
//					m_debugVisualizeShader->setModelMatrix(cylinderModel);
//
//					auto& cylinderMesh = BuiltInAssets::getByName<ModelAsset>(SGE_MESH_CYLINDER);
//					auto  cylinderVao = cylinderMesh.resource()->getPrimaryMesh()->getVAO();
//					auto& sphereMesh = BuiltInAssets::getByName<ModelAsset>(SGE_MESH_SPHERE);
//					auto  sphereVao = sphereMesh.resource()->getPrimaryMesh()->getVAO();
//
//					// Top cap sphere
//					topSphereModel = glm::translate(topSphereModel, glm::vec3(0.0f, halfHeight, 0.0f));
//					topSphereModel = glm::scale(topSphereModel, glm::vec3(radius * 2));
//					m_debugVisualizeShader->setModelMatrix(topSphereModel);
//					RenderCommand::draw(sphereVao);
//
//					// Bottom cap sphere
//					bottomSphereModel = glm::translate(bottomSphereModel, glm::vec3(0.0f, -halfHeight, 0.0f));
//					bottomSphereModel = glm::scale(bottomSphereModel, glm::vec3(radius * 2));
//					m_debugVisualizeShader->setModelMatrix(bottomSphereModel);
//					RenderCommand::draw(sphereVao);
//
//					// Cylinder body
//					m_debugVisualizeShader->setModelMatrix(cylinderModel);
//					RenderCommand::draw(cylinderVao);
//				}
//
//				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
//				glDisable(GL_POLYGON_OFFSET_LINE);
//
//				glEnable(GL_DEPTH_TEST);
//
//				glPopDebugGroup();
//			}
//		}
//
//		// Render physics collider
//		for (auto&& [entity, transform, controller] : scene->getRegistry().get().view<Transformation, PlayerController>().each())
//		{
//			if ((entity_id)entity == selectedObject)
//			{
//				glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Physics Debug");
//
//				m_debugVisualizeShader->use();
//
//				m_debugVisualizeShader->setViewMatrix(graphics->view);
//				m_debugVisualizeShader->setProjectionMatrix(graphics->projection);
//				m_debugVisualizeShader->setUniformValue("color", glm::vec3(0, 1, 0));
//
//				glDisable(GL_DEPTH_TEST);
//
//				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
//				glEnable(GL_POLYGON_OFFSET_LINE);
//				glPolygonOffset(-1.0, -1.0);
//				glLineWidth(1); // Size in pixels
//
//				glm::mat4 model = transform.getWorldTransformation();
//				model = model * glm::translate(glm::mat4(1.0f), controller.offset);
//
//				float radius = controller.radius;
//				float height = controller.height;
//				glm::mat4 topSphereModel = model;
//				glm::mat4 bottomSphereModel = model;
//				glm::mat4 cylinderModel = model;
//				cylinderModel = glm::scale(cylinderModel, glm::vec3(radius * 2, height, radius * 2));
//				m_debugVisualizeShader->setModelMatrix(cylinderModel);
//
//				auto& cylinderMesh = BuiltInAssets::getByName<ModelAsset>(SGE_MESH_CYLINDER);
//				auto  cylinderVao = cylinderMesh.resource()->getPrimaryMesh()->getVAO();
//				auto& sphereMesh = BuiltInAssets::getByName<ModelAsset>(SGE_MESH_SPHERE);
//				auto  sphereVao = sphereMesh.resource()->getPrimaryMesh()->getVAO();
//
//				// Top cap sphere
//				topSphereModel = glm::translate(topSphereModel, glm::vec3(0.0f, height * .5f, 0.0f));
//				topSphereModel = glm::scale(topSphereModel, glm::vec3(radius * 2));
//				m_debugVisualizeShader->setModelMatrix(topSphereModel);
//				RenderCommand::draw(sphereVao);
//
//				// Bottom cap sphere
//				bottomSphereModel = glm::translate(bottomSphereModel, glm::vec3(0.0f, -height * .5f, 0.0f));
//				bottomSphereModel = glm::scale(bottomSphereModel, glm::vec3(radius * 2));
//				m_debugVisualizeShader->setModelMatrix(bottomSphereModel);
//				RenderCommand::draw(sphereVao);
//
//				// Cylinder body
//				m_debugVisualizeShader->setModelMatrix(cylinderModel);
//				RenderCommand::draw(cylinderVao);
//
//
//				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
//				glDisable(GL_POLYGON_OFFSET_LINE);
//
//				glEnable(GL_DEPTH_TEST);
//
//				glPopDebugGroup();
//			}
//		}
//
//		// Render WireframeGrid
//		if (Engine::get()->getConfig().renderConfig.renderWireframeGrid)
//		{
//			glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Wireframe Grid");
//
//			m_wireframeGrid->shader->use();
//			m_wireframeGrid->shader->setModelMatrix(glm::mat4(1.0));
//			m_wireframeGrid->shader->setViewMatrix(graphics->view);
//			m_wireframeGrid->shader->setProjectionMatrix(graphics->projection);
//			m_wireframeGrid->shader->setUniformValue("color", glm::vec3(0.6, 0.6, 0.6));
//
//			m_wireframeGrid->vao->Bind();
//
//			glLineWidth(2); // Size in pixels
//			glDrawArrays(GL_LINES, 0, m_wireframeGrid->vao->GetVerticesCount());
//
//			glPopDebugGroup();
//		}
//
//		if (Engine::get()->getConfig().renderConfig.renderPhysicsDebugPass)
//		{
//			Engine::get()->getPhysicsSystem()->visualizePhysicsShapeDebug(scene);
//		}
//
//		if (Engine::get()->getConfig().renderConfig.renderNonOpaquePass)
//		{
//			glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Non Opaque render pass");
//			glEnable(GL_BLEND);
//			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//			Engine::get()->getForwardRenderer().renderSceneNonOpaque(scene);
//			glDisable(GL_BLEND);
//			glPopDebugGroup();
//		}
//
//		if (Engine::get()->getConfig().renderConfig.renderDebugDataPass)
//		{
//			glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Debug Data render pass");
//			Engine::get()->getForwardRenderer().renderDebugData(scene);
//			glPopDebugGroup();
//		}
//
//		{
//			glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Orientation Gizmo");
//
//			// Orientation gizmo in bottom-left corner
//			glViewport(10, 10, 100, 100);  // Small 100x100 viewport at bottom-left
//			glClear(GL_DEPTH_BUFFER_BIT);  // Clear depth so gizmo is always on top
//
//			// Use orthographic or perspective projection
//			glm::mat4 gizmoProjection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 10.0f);
//			graphics->projection = gizmoProjection;
//
//			// Use ONLY the rotation part of your view matrix (no translation)
//			glm::mat3 rotation = glm::mat3(graphics->view);
//			glm::mat4 gizmoView = glm::mat4(rotation);
//			gizmoView = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, -3)) * gizmoView;
//			graphics->view = gizmoView;
//
//			// Draw three lines from origin
//			DebugHelper::getInstance().drawLine(glm::vec3(0), glm::vec3(1, 0, 0), glm::vec3(1, 0, 0), 2);
//			DebugHelper::getInstance().drawLine(glm::vec3(0), glm::vec3(0, 1, 0), glm::vec3(0, 1, 0), 2);
//			DebugHelper::getInstance().drawLine(glm::vec3(0), glm::vec3(0, 0, 1), glm::vec3(0, 0, 1), 2);
//
//			// Reset viewport
//			RenderCommand::setViewport(viewport.x, viewport.y, viewport.w, viewport.h);
//
//			glPopDebugGroup();
//		}
//
//		// Render UI
//		glEnable(GL_BLEND);
//		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//		m_UIShader->use();
//		m_UIShader->setProjectionMatrix(m_defaultUIProjection);
//		auto vao = BuiltInAssets::getByName<ModelAsset>(SGE_MESH_QUAD).resource()->getPrimaryMesh()->getVAO();
//
//		for (auto&& [entity, image] : scene->getRegistry().get().view<ImageComponent>().each())
//		{
//			Entity entityhandler{ entity, m_registry.get() };
//			graphics->entity = entityhandler;
//			image.image.resource()->bind();
//			image.image.resource()->setSlot(0);
//
//			glm::mat4 model = glm::mat4(1.0f);
//			model = glm::translate(model, glm::vec3(image.position, 0.0f));
//
//			model = glm::translate(model, glm::vec3(0.5f * image.size.x, 0.5f * image.size.y, 0.0f));
//			model = glm::rotate(model, glm::radians(image.rotate), glm::vec3(0.0f, 0.0f, 1.0f));
//			model = glm::translate(model, glm::vec3(-0.5f * image.size.x, -0.5f * image.size.y, 0.0f));
//
//			model = glm::scale(model, glm::vec3(image.size, 1.0f));
//
//			m_UIShader->setUniformValue("model", model);
//
//			RenderCommand::draw(vao);
//		}
//
//		glDisable(GL_BLEND);
//
//
//
//
//#if 0 // TODO make use of this quite usefull camera frustum debug code
//		auto gameCameraTransform = getRenderView("Game View")->getCamera().getComponent<Transformation>();
//		auto gameCameraComponent = getRenderView("Game View")->getCamera().getComponent<CameraComponent>();
//		auto gameCameraView = glm::lookAt(gameCameraTransform.getWorldPosition(), gameCameraTransform.getWorldPosition() + gameCameraComponent.front, gameCameraComponent.up);
//
//		glm::mat4 view = gameCameraView;
//		glm::mat4 projection = graphics->projection;
//		glm::mat4 invViewProj = glm::inverse(projection * view);
//
//		// NDC cube corners
//		static glm::vec3 ndcCorners[8] = {
//			{-1, -1, -1}, {1, -1, -1},
//			{-1,  1, -1}, {1,  1, -1},
//			{-1, -1,  1}, {1, -1,  1},
//			{-1,  1,  1}, {1,  1,  1}
//		};
//
//		// Transform to world space
//		static glm::vec3 worldCorners[8];
//		for (int i = 0; i < 8; ++i) {
//			glm::vec4 p = invViewProj * glm::vec4(ndcCorners[i], 1.0f);
//			worldCorners[i] = glm::vec3(p) / p.w;
//		}
//
//
//		auto& debug = DebugHelper::getInstance();
//
//		// Near plane
//		debug.drawLine(worldCorners[0], worldCorners[1]);
//		debug.drawLine(worldCorners[1], worldCorners[3]);
//		debug.drawLine(worldCorners[3], worldCorners[2]);
//		debug.drawLine(worldCorners[2], worldCorners[0]);
//
//		// Far plane
//		debug.drawLine(worldCorners[4], worldCorners[5]);
//		debug.drawLine(worldCorners[5], worldCorners[7]);
//		debug.drawLine(worldCorners[7], worldCorners[6]);
//		debug.drawLine(worldCorners[6], worldCorners[4]);
//
//		// Connect near to far
//		for (int i = 0; i < 4; ++i) {
//			debug.drawLine(worldCorners[i], worldCorners[i + 4]);
//		}
//
//#endif
//
//		if (Engine::get()->getConfig().renderConfig.renderPostProcess)
//		{
//			glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Post Process render pass");
//
//			// Render Post Process Effects
//			for (auto&& [entity, postProcess] : scene->getRegistry().get().view<PostProcessComponent>().each())
//			{
//				TextureResourceRef renderTargetTexture = graphics->renderView->getRenderTargetTexture();
//				renderView->swapToAdditionalTarget();
//				renderView->bind();
//				RenderCommand::clear();
//				glDisable(GL_DEPTH_TEST);
//				// TODO assert post process shader
//
//				// bind shader
//				auto shader = postProcess.shader.resource();
//
//				if (shader.isEmpty())
//				{
//					continue;
//				}
//
//				shader->use();
//
//				// read texture from graphics FBO
//				shader->setTextureInShader(renderTargetTexture, "MainTexture", 0); //todo check slot
//
//				//shader->setModelMatrix(glm::mat4(1.0));
//				//shader->setViewMatrix(graphics->view);
//				//shader->setProjectionMatrix(graphics->projection);
//
//				auto viewport = renderView->getViewport();
//				shader->setUniformValue("screenSize", glm::vec2(viewport.w, viewport.h));
//				glm::vec2 texelSize = glm::vec2(1.0 / viewport.w, 1.0 / viewport.h);
//				shader->setUniformValue("texelSize", texelSize);
//
//				//shader->setUniformValue("cameraPos", graphics->cameraPos);
//				//shader->setUniformValue("cameraLookAt", primaryCamera.front);
//
//				// bind mesh
//				auto vao = BuiltInAssets::getByName<ModelAsset>(SGE_MESH_QUAD).resource()->getPrimaryMesh()->getVAO();
//
//				// in frag shader i need access to mesh extentes & main texture -> set uniforms
//
//				// draw
//				RenderCommand::draw(vao);
//
//				renderView->swapBackToMainTargetWithCopy();
//				renderView->bind();
//				glEnable(GL_DEPTH_TEST);
//			}
//
//			glPopDebugGroup();
//		}
//
//		glPopDebugGroup();
//	}
//
//
//
//
//	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
//	//glDisable(GL_POLYGON_OFFSET_LINE);
//
//	//auto& debug = DebugHelper::getInstance();
//
//	//static float offsetX = 0.0;
//	//static float offsetY = 0.0;
//
//	//offsetX = fmod(offsetX + .01f, 1.0f);
//	//offsetY = fmod(offsetY + .01f, 1.0f);
//
//	//for (auto& [e, terrain] : Engine::get()->getContext()->getActiveScene()->getRegistry().getRegistry().view<Terrain>().each())
//	//{
//
//	//	for (float i = -terrain.getWidth() / 2 + offsetX; i < terrain.getWidth() / 2; i++)
//	//	{
//	//		for (float j = -terrain.getHeight() / 2 + offsetY; j < terrain.getHeight() / 2; j++)
//	//		{
//	//			float height = terrain.getHeightAtPoint(i, j);
//	//			debug.drawPoint({ i, height, j });
//	//		}
//	//	}
//	//}
//
//
//
//
//	//for (const auto& cb : m_renderCallbacks[RenderPhase::POST_RENDER_BEGIN])
//	//{
//	//	cb();
//	//}
//
//	//for (const auto& cb : m_renderCallbacks[RenderPhase::POST_RENDER_END])
//	//{
//	//	cb();
//	//}
//}
