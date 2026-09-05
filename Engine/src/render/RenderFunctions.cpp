#include "render/RenderFunctions.h"

#include <map>

#include "gl/glew.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "core/Engine.h"
#include "render/Graphics.h"
#include "render/RenderCommand.h"
#include "render/Material.h"
#include "render/MaterialData.h"
#include "render/Shader.h"
#include "runtime/Scene.h"
#include "runtime/Entity.h"
#include "component/MeshRendererComponent.h"
#include "component/Transformation.h"
#include "component/ObjectComponent.h"
#include "memory/BuiltInResources.h"
#include "memory/BuiltInAssets.h"
#include "render/RenderView.h"
#include "render/VertexArrayObject.h"
#include "geometry/Model.h"
#include "geometry/Mesh.h"
#include "animation/Animator.h"

bool RenderFunctions::prepareMeshForRender(Mesh* mesh, const Entity& entityHandler)
{
	auto graphics = Engine::get()->getSubSystem<Graphics>();

	auto& meshRenderer = entityHandler.getComponent<MeshRendererComponent>();

	graphics->mesh = mesh;
	auto& transform = entityHandler.getComponent<Transformation>();
	glm::mat4 modelTransform = transform.getWorldTransformation() * mesh->getRestTransform();
	graphics->model = modelTransform;

	AABB& aabb = mesh->getAABB();
	aabb.transform(modelTransform);

	if (!aabb.isOnFrustum(*graphics->frustum))
	{
		return false;
	}

	//DebugHelper::getInstance().drawAABB(aabb);

	auto matIndex = mesh->getMaterialIndex();
	graphics->material = meshRenderer.at(matIndex);

	if (graphics->material.isEmpty())
	{
		graphics->material = BuiltInAssets::getByName<MaterialAsset>(SGE_MATERIAL_DEFAULT).resource();
	}

	return true;
}

bool RenderFunctions::prepareEntityForRender(const Entity& entityHandler)
{
	auto graphics = Engine::get()->getSubSystem<Graphics>();

	// Display name
	std::string name = entityHandler.getComponent<ObjectComponent>().name;
	logTrace("About to render Entity {}", name);

	// Apply animation logic
	auto animator = entityHandler.tryGetComponent<Animator>();
	if (!animator || !animator->hasActiveAnimation())
	{
		graphics->shader->setUniformValue("isAnimated", false);
	}
	else
	{
		auto& meshRenderer = entityHandler.getComponent<MeshRendererComponent>();

		std::vector<glm::mat4> finalBoneMatrices;
		animator->getFinalBoneMatrices(meshRenderer.mesh.resource(), finalBoneMatrices);
		for (int i = 0; i < finalBoneMatrices.size(); ++i)
		{
			graphics->shader->setUniformValue("finalBonesMatrices[" + std::to_string(i) + "]", finalBoneMatrices[i]);
		}

		graphics->shader->setUniformValue("isAnimated", true);
	}

	return true;
}


void RenderFunctions::drawForwardScene(Scene* scene)
{
	auto graphics = Engine::get()->getSubSystem<Graphics>();

	auto shader = BuiltInResources::get<Shader>(SGE_RESOURCE_SHADER_FORWARD_PBR);
	shader->use();

	graphics->shader->setViewMatrix(graphics->view);
	graphics->shader->setProjectionMatrix(graphics->projection);
	graphics->shader->bindUniformBlockToBindPoint("Time", 0);
	graphics->shader->bindUniformBlockToBindPoint("Lights", 1);
	graphics->shader->setTextureInShader(graphics->irradianceMap, "gIrradianceMap", 6);
	graphics->shader->setTextureInShader(graphics->prefilterEnvMap, "gPrefilterEnvMap", 7);
	graphics->shader->setTextureInShader(graphics->brdfLUT, "gBRDFIntegrationLUT", 8);
	graphics->shader->setUniformValue("cameraPos", graphics->cameraPos);

	glEnable(GL_DEPTH_TEST);
	graphics->renderView->bind();

	for (auto&& [entity, meshRenderer, transform, obj] :
		scene->getRegistry().getRegistry().view<MeshRendererComponent, Transformation, ObjectComponent>().each())
	{
		if (meshRenderer.renderTechnique != MeshRendererComponent::RenderTechnique::Forward)
			continue;

		Entity entityHandler{ entity, &scene->getRegistry() };

		prepareEntityForRender(entityHandler);

		for (auto& mesh : meshRenderer.mesh.resource()->getMeshes())
		{
			if (!prepareMeshForRender(mesh.get(), entityHandler))
			{
				continue;
			}

			graphics->shader->setModelMatrix(graphics->model);
			graphics->material->use();

			// Draw
			RenderCommand::draw(mesh->getVAO());
		}
	}
}

void RenderFunctions::drawTransparentScene(Scene* scene)
{
	auto graphics = Engine::get()->getSubSystem<Graphics>();

	graphics->renderView->bind();

	std::map<float, Entity> transparentEntities;

	auto& camera = graphics->renderView->getCamera();
	auto& camTransform = camera.getComponent<Transformation>();
	auto& camForward = camTransform.getForward();

	for (auto&& [entity, mesh, transform, obj] :
		scene->getRegistry().getRegistry().view<MeshRendererComponent, Transformation, ObjectComponent>().each())
	{
		Entity entityHandler{ entity, &scene->getRegistry() };

		auto& meshRenderer = entityHandler.getComponent<MeshRendererComponent>();

		for (auto& mesh : meshRenderer.mesh.resource()->getMeshes())
		{
			float distance = glm::dot(transform.getWorldPosition(), camForward);

			// object is behind the camera
			if (distance < 0)
			{
				//continue; // TODO fix
			}

			transparentEntities[distance] = entityHandler;
		}
	}

	graphics->shader = BuiltInResources::get<Shader>(SGE_RESOURCE_SHADER_FORWARD_PBR);
	graphics->shader->use();
	graphics->shader->setViewMatrix(graphics->view);
	graphics->shader->setProjectionMatrix(graphics->projection);
	graphics->shader->bindUniformBlockToBindPoint("Time", 0);
	graphics->shader->bindUniformBlockToBindPoint("Lights", 1);
	graphics->shader->setTextureInShader(graphics->irradianceMap, "gIrradianceMap", 6);
	graphics->shader->setTextureInShader(graphics->prefilterEnvMap, "gPrefilterEnvMap", 7);
	graphics->shader->setTextureInShader(graphics->brdfLUT, "gBRDFIntegrationLUT", 8);
	graphics->shader->setUniformValue("cameraPos", graphics->cameraPos);

	auto iter = transparentEntities.rbegin();
	while (iter != transparentEntities.rend())
	{
		Entity& entityHandler = iter->second;

		std::string name = entityHandler.getComponent<ObjectComponent>().name;
		std::string captionGPU = "About to render: '" + name + "'";
		glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, captionGPU.c_str());

		prepareEntityForRender(entityHandler);

		graphics->entity = entityHandler;
		
		for (auto& mesh : entityHandler.getComponent<MeshRendererComponent>().mesh.resource()->getMeshes())
		{
			if (!prepareMeshForRender(mesh.get(), entityHandler))
			{
				continue;
			}

			// Only render transparent objects
			if (graphics->material->getRenderMode() != MaterialRenderMode::Transparent)
			{
				continue;
			}

			// draw model
			glm::mat3 transposeInverseModelMatrix = glm::mat3(glm::transpose(glm::inverse(graphics->model)));
			graphics->shader->setUniformValue("transposeInverseModelMatrix", transposeInverseModelMatrix);

			graphics->shader->setModelMatrix(graphics->model);

			graphics->material->use();

			std::string captionSubmeshGPU = "About to render submesh: '" + mesh->getName() + "' using material: '" + graphics->material->getName() + "'";
			glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, captionSubmeshGPU.c_str());

			// Draw
			RenderCommand::draw(graphics->mesh->getVAO());

			glPopDebugGroup();
		}

		glPopDebugGroup();

		iter++;
	}
}

void RenderFunctions::drawDebugData(Scene* scene)
{
	auto graphics = Engine::get()->getSubSystem<Graphics>();

	graphics->shader = BuiltInResources::get<Shader>(SGE_RESOURCE_SHADER_DEBUG_DATA);
	graphics->shader->use();

	graphics->shader->setViewMatrix(graphics->view);
	graphics->shader->setProjectionMatrix(graphics->projection);
	graphics->shader->bindUniformBlockToBindPoint("Time", 0);
	graphics->shader->setUniformValue("cameraPos", graphics->cameraPos);

	glEnable(GL_DEPTH_TEST);
	graphics->renderView->bind();

	for (auto&& [entity, meshRenderer, transform, obj] :
		scene->getRegistry().getRegistry().view<MeshRendererComponent, Transformation, ObjectComponent>().each())
	{
		Entity entityHandler{ entity, &scene->getRegistry() };
		std::string name = entityHandler.getComponent<ObjectComponent>().name;
		logTrace("About to display debug data for {}", name);

		for (auto& mesh : meshRenderer.mesh.resource()->getMeshes())
		{
			if (!prepareMeshForRender(mesh.get(), entityHandler))
			{
				continue;
			}

			graphics->shader->setModelMatrix(graphics->model);

			// Draw
			RenderCommand::draw(mesh->getVAO());
		}
	}
}

void RenderFunctions::drawSceneUsingCustomShader(Scene* scene)
{
	auto graphics = Engine::get()->getSubSystem<Graphics>();

	glEnable(GL_DEPTH_TEST);
	graphics->renderView->bind();

	for (auto&& [entity, meshRenderer, transform, obj] :
		scene->getRegistry().getRegistry().view<MeshRendererComponent, Transformation, ObjectComponent>().each())
	{
		Entity entityHandler{ entity, &scene->getRegistry() };
		std::string name = entityHandler.getComponent<ObjectComponent>().name;
		logTrace("About to render '{}' using Custom Shader pass", name);

		for (auto& mesh : meshRenderer.mesh.resource()->getMeshes())
		{
			if (!prepareMeshForRender(mesh.get(), entityHandler))
			{
				continue;
			}

			if (graphics->material->getRenderMode() != MaterialRenderMode::Custom)
				continue;

			// draw model

			graphics->shader = graphics->material->getActiveShader();
			graphics->shader->use();
			// Ensure per-entity uniforms (including animation) are set for custom shaders
			graphics->entity = entityHandler;
			prepareEntityForRender(entityHandler);
			glm::mat3 transposeInverseModelMatrix = glm::mat3(glm::transpose(glm::inverse(graphics->model)));
			graphics->shader->setUniformValue("transposeInverseModelMatrix", transposeInverseModelMatrix);

			graphics->shader = graphics->material->getActiveShader();
			graphics->shader->setModelMatrix(graphics->model);
			graphics->shader->setViewMatrix(graphics->view);
			graphics->shader->setProjectionMatrix(graphics->projection);
			graphics->material->use();

			graphics->shader->bindUniformBlockToBindPoint("Time", 0);
			graphics->shader->bindUniformBlockToBindPoint("Lights", 1);
			graphics->shader->setTextureInShader(graphics->irradianceMap, "gIrradianceMap", 6);
			graphics->shader->setTextureInShader(graphics->prefilterEnvMap, "gPrefilterEnvMap", 7);
			graphics->shader->setTextureInShader(graphics->brdfLUT, "gBRDFIntegrationLUT", 8);

			graphics->shader->setUniformValue("cameraPos", graphics->cameraPos);

			// Draw
			RenderCommand::draw(mesh->getVAO());
		}
	}
}

void RenderFunctions::drawGeometryToGBuffer(Scene* scene)
{
	auto graphics = Engine::get()->getSubSystem<Graphics>();

	glEnable(GL_DEPTH_TEST);

	if (graphics->renderMode == RenderMode::WIREFRAME)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glEnable(GL_POLYGON_OFFSET_LINE);
		glPolygonOffset(-1.0, -1.0);
		glLineWidth(1); // Size in pixels
	}

	graphics->shader = BuiltInResources::get<Shader>(SGE_RESOURCE_SHADER_DEFFERED_PBR_GEOM);
	graphics->shader->use();

	graphics->shader->setViewMatrix(graphics->view);
	graphics->shader->setProjectionMatrix(graphics->projection);
	graphics->shader->bindUniformBlockToBindPoint("Time", 0);
	graphics->shader->bindUniformBlockToBindPoint("Lights", 1);

	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "G-Buffer pass");

	// Render all objects
	for (auto&& [entity, meshRenderer, transform, obj] :
		scene->getRegistry().getRegistry().view<MeshRendererComponent, Transformation, ObjectComponent>().each())
	{
		if (meshRenderer.renderTechnique != MeshRendererComponent::RenderTechnique::Deferred)
			continue;

		Entity entityHandler{ entity, &scene->getRegistry() };
		graphics->entity = entityHandler;

		std::string name = entityHandler.getComponent<ObjectComponent>().name;
		std::string captionGPU = "About to render Entity: '" + name + "'";
		glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, captionGPU.c_str());

		prepareEntityForRender(entityHandler);

		for (auto& mesh : meshRenderer.mesh.resource()->getMeshes())
		{
			if (!prepareMeshForRender(mesh.get(), entityHandler))
			{
				continue;
			}

			// Only render Opaque objects
			if (graphics->material->getRenderMode() != MaterialRenderMode::Opaque)
			{
				continue;
			}

			std::string captionSubmeshGPU = "About to render submesh: '" + mesh->getName() + "' using material: '" + graphics->material->getName() + "'";
			glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, captionSubmeshGPU.c_str());

			// draw model

			graphics->shader->setModelMatrix(graphics->model);

			graphics->material->use();

			// Draw
			RenderCommand::draw(graphics->mesh->getVAO());

			glPopDebugGroup();
		}

		glPopDebugGroup();

	};

	glPopDebugGroup();
}

void RenderFunctions::drawInstancedGeometryToGBuffer(Scene* scene)
{
	auto graphics = Engine::get()->getSubSystem<Graphics>();

	glEnable(GL_DEPTH_TEST);

	graphics->shader = BuiltInResources::get<Shader>(SGE_RESOURCE_SHADER_DEFFERED_PBR_GEOM); //todo change to instanced
	graphics->shader->use();

	graphics->shader->setViewMatrix(graphics->view);
	graphics->shader->setProjectionMatrix(graphics->projection);
	graphics->shader->bindUniformBlockToBindPoint("Time", 0);
	graphics->shader->bindUniformBlockToBindPoint("Lights", 1);

	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "G-Buffer instanced pass");

	struct InstancedRenderData
	{
		std::shared_ptr<Mesh> mesh;
		MaterialResourceRef material;
		std::vector<glm::mat4> models;
	};

	std::unordered_map<unsigned int, InstancedRenderData> instancedRenderData;

	// Render all objects
	for (auto&& [entity, meshRenderer, transform, obj] :
		scene->getRegistry().getRegistry().view<MeshRendererComponent, Transformation, ObjectComponent>().each())
	{
		if (!meshRenderer.isInstanced)
			continue;

		for (auto& mesh : meshRenderer.mesh.resource()->getMeshes())
		{
			Entity entityHandler{ entity, &scene->getRegistry() };
			if (!prepareMeshForRender(mesh.get(), entityHandler))
			{
				continue;
			}

			if (graphics->material->getRenderMode() != MaterialRenderMode::Opaque)
			{
				continue;
			}

			auto vaoID = mesh->getVAO()->getID();

			auto it = instancedRenderData.find(vaoID);
			if (it == instancedRenderData.end())
			{
				InstancedRenderData data;
				data.mesh = mesh;
				data.material = graphics->material;
				data.models.push_back(graphics->model);
				instancedRenderData.emplace(vaoID, std::move(data));
			}
			else
			{
				it->second.models.push_back(graphics->model);
			}

			//get animations and store in anim ssbo by key
		}

	};

	for(auto& [_, ird] : instancedRenderData)
	{
		graphics->instancedModelBuffer.setData(sizeof(glm::mat4) * ird.models.size(), ird.models.data());
		RenderCommand::drawInstanced(ird.mesh->getVAO(), ird.models.size());

	}


	glPopDebugGroup();
}

void RenderFunctions::drawLightPass(const GBuffer& gBuffer)
{
	auto graphics = Engine::get()->getSubSystem<Graphics>();

	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Light pass");

	// bind textures
	// Todo solve slots issue
	ShaderResourceRef lightPassShaderResource = BuiltInResources::get<Shader>(SGE_RESOURCE_SHADER_DEFFERED_PBR_LIGHT);
	lightPassShaderResource->use();
	lightPassShaderResource->setTextureInShader(gBuffer.getTexture(GBuffer::Attachment::Position), "gPosition", 0);
	lightPassShaderResource->setTextureInShader(gBuffer.getTexture(GBuffer::Attachment::Normal), "gNormal", 1);
	lightPassShaderResource->setTextureInShader(gBuffer.getTexture(GBuffer::Attachment::Albedo), "gAlbedo", 2);
	lightPassShaderResource->setTextureInShader(gBuffer.getTexture(GBuffer::Attachment::MRA), "gMRA", 3);
	lightPassShaderResource->setTextureInShader(graphics->irradianceMap, "gIrradianceMap", 4);
	lightPassShaderResource->setTextureInShader(graphics->prefilterEnvMap, "gPrefilterEnvMap", 5);
	lightPassShaderResource->setTextureInShader(graphics->brdfLUT, "gBRDFIntegrationLUT", 6);
	lightPassShaderResource->setTextureInShader(graphics->shadowMap, "gShadowMap", 7);
	lightPassShaderResource->setTextureInShader(graphics->ssaoTexture, "gSSAOColorBuffer", 8); 
	lightPassShaderResource->setTextureInShader(gBuffer.getTexture(GBuffer::Attachment::Tangent), "gTangnet", 9);
	lightPassShaderResource->bindUniformBlockToBindPoint("Time", 0);
	lightPassShaderResource->bindUniformBlockToBindPoint("Lights", 1);
	lightPassShaderResource->setUniformValue("cameraPos", graphics->cameraPos);
	lightPassShaderResource->setUniformValue("lightSpaceMatrix", graphics->lightSpaceMatrix);
	lightPassShaderResource->setUniformValue("useSSAO", graphics->useSSAO);

	graphics->renderView->bind();

	{
		// render to quad
		auto vao = BuiltInAssets::getByName<ModelAsset>(SGE_MESH_QUAD).resource()->getPrimaryMesh()->getVAO();
		RenderCommand::draw(vao);
	}

	glPopDebugGroup();
}
