#include "render/IndirectRenderer.h"

#include "memory/ResourceRef.h"
#include "gl/glew.h"
#include "core/Engine.h"
#include "core/Window.h"
#include "core/Logger.h"
#include "component/Component.h"
#include "component/Transformation.h"
#include "render/ScreenQuad.h"
#include "runtime/Scene.h"
#include "render/Material.h"
#include "core/Random.h"
#include "render/RenderCommand.h"
#include "render/VertexArrayOBject.h"
#include "runtime/Context.h"
#include "animation/Animator.h"
#include "geometry/Model.h"
#include "render/Graphics.h"
#include "core/System.h"
#include "geometry/ShapeFactory.h"
#include "render/RenderView.h"
#include "utils/DebugHelper.h"
#include "component/MeshRendererComponent.h"

#include "component/ObjectComponent.h"
#include "component/RenderableComponent.h"
#include "memory/BuiltInAssets.h"
#include "memory/BuiltInResources.h"

static float lerp(float a, float b, float t)
{
	return a + t * (b - a);
}

bool IndirectRenderer::setupGBuffer(int width, int height)
{
	m_gBuffer.bind();

	// Generate Texture for Position data
	m_positionTexture = Texture::createTexture(width, height, 3, TextureInternalFormat::RGBA16F, TextureFormat::RGBA, TextureType::FLOAT);
	m_gBuffer.attachTexture(m_positionTexture.get()->getID(), GL_COLOR_ATTACHMENT0);

	// Generate Texture for Normal data
	m_normalTexture = Texture::createTexture(width, height, 3, TextureInternalFormat::RGBA16F, TextureFormat::RGBA, TextureType::FLOAT);
	m_gBuffer.attachTexture(m_normalTexture.get()->getID(), GL_COLOR_ATTACHMENT1);

	// Generate Texture for Albedo
	m_albedoTexture = Texture::createTexture(width, height, 3, TextureInternalFormat::RGBA, TextureFormat::RGBA, TextureType::UNSIGNED_BYTE);
	m_gBuffer.attachTexture(m_albedoTexture.get()->getID(), GL_COLOR_ATTACHMENT2);

	// Generate Texture for MRA
	m_MRATexture = Texture::createTexture(width, height, 3, TextureInternalFormat::RGBA, TextureFormat::RGBA, TextureType::UNSIGNED_BYTE);
	m_gBuffer.attachTexture(m_MRATexture.get()->getID(), GL_COLOR_ATTACHMENT3);

	// Generate Texture for Position ViewSpace data
	m_positionTextureVS = Texture::createTexture(width, height, 3, TextureInternalFormat::RGBA16F, TextureFormat::RGBA, TextureType::FLOAT);
	m_gBuffer.attachTexture(m_positionTextureVS.get()->getID(), GL_COLOR_ATTACHMENT4);

	// Generate Texture for Normal ViewSpace data
	m_normalTextureVS = Texture::createTexture(width, height, 3, TextureInternalFormat::RGBA16F, TextureFormat::RGBA, TextureType::FLOAT);
	m_gBuffer.attachTexture(m_normalTextureVS.get()->getID(), GL_COLOR_ATTACHMENT5);

	// Generate Texture for Tangent data
	m_TangentTexture = Texture::createTexture(width, height, 3, TextureInternalFormat::RGBA16F, TextureFormat::RGBA, TextureType::FLOAT);
	m_gBuffer.attachTexture(m_TangentTexture.get()->getID(), GL_COLOR_ATTACHMENT6);

	unsigned int attachments[7] = {
		GL_COLOR_ATTACHMENT0,
		GL_COLOR_ATTACHMENT1,
		GL_COLOR_ATTACHMENT2,
		GL_COLOR_ATTACHMENT3,
		GL_COLOR_ATTACHMENT4,
		GL_COLOR_ATTACHMENT5,
		GL_COLOR_ATTACHMENT6
	};
	glDrawBuffers(7, attachments);

	// Create RBO and attach to FBO
	m_renderBuffer = RenderBufferObject(width, height);
	m_gBuffer.attachRenderBuffer(m_renderBuffer.GetID(), FrameBufferObject::AttachmentType::Depth_Stencil);

	if (!m_gBuffer.isComplete())
	{
		logError("FBO is not complete!");
		return false;
	}

	m_gBuffer.unbind();

	return true;
}

bool IndirectRenderer::init()
{
	glGenBuffers(1, &m_indirectBuffer);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_indirectBuffer);
	glBufferData(GL_DRAW_INDIRECT_BUFFER, 1000 * sizeof(RenderData::DrawCommand), nullptr, GL_DYNAMIC_DRAW);

	auto width = Engine::get()->getWindow()->getWidth();
	auto height = Engine::get()->getWindow()->getHeight();

	setupGBuffer(width, height);

	// Generate screen quad
	m_quad = Engine::get()->getSubSystem<Assets>()->getAssetFromName(SGE_MESH_QUAD).as<ModelAsset>().resource();

	return true;
}

void IndirectRenderer::renderScene(Scene* scene)
{
	auto graphics = Engine::get()->getSubSystem<Graphics>();

	glBindFramebuffer(GL_FRAMEBUFFER, getGBuffer().getID());
	RenderCommand::clear();

	glEnable(GL_DEPTH_TEST);

	if (graphics->renderMode == RenderMode::WIREFRAME)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glEnable(GL_POLYGON_OFFSET_LINE);
		glPolygonOffset(-1.0, -1.0);
		glLineWidth(1); // Size in pixels
	}

	graphics->shader = BuiltInResources::get<Shader>(SGE_RESOURCE_SHADER_INDIRECT_PBR_GEOM);
	graphics->shader->use();

	graphics->shader->setViewMatrix(graphics->view);
	graphics->shader->setProjectionMatrix(graphics->projection);
	graphics->shader->bindUniformBlockToBindPoint("Time", 0);
	graphics->shader->bindUniformBlockToBindPoint("Lights", 1);

	// Render all objects
	for (auto&& [entity, meshRenderer, transform, obj] :
		scene->getRegistry().getRegistry().view<MeshRendererComponent, Transformation, ObjectComponent>().each())
	{
		if (meshRenderer.renderTechnique != MeshRendererComponent::RenderTechnique::Indirect)
			continue;

		Entity entityHandler{ entity, &scene->getRegistry() };
		graphics->entity = entityHandler;

		// Apply animation logic
		//auto animator = entityHandler.tryGetComponent<Animator>();
		//if (!animator || !animator->hasActiveAnimation())
		//{
		//	graphics->shader->setUniformValue("isAnimated", false);
		//}
		//else
		//{
		//	auto& meshRenderer = entityHandler.getComponent<MeshRendererComponent>();

		//	std::vector<glm::mat4> finalBoneMatrices;
		//	animator->getFinalBoneMatrices(meshRenderer.mesh.resource(), finalBoneMatrices);
		//	for (int i = 0; i < finalBoneMatrices.size(); ++i)
		//	{
		//		graphics->shader->setUniformValue("finalBonesMatrices[" + std::to_string(i) + "]", finalBoneMatrices[i]);
		//	}

		//	graphics->shader->setUniformValue("isAnimated", true);
		//}



		for (auto& mesh : meshRenderer.mesh.resource()->getMeshes())
		{
			auto graphics = Engine::get()->getSubSystem<Graphics>();

			auto& meshRenderer = entityHandler.getComponent<MeshRendererComponent>();

			graphics->mesh = mesh.get();
			auto& transform = entityHandler.getComponent<Transformation>();
			glm::mat4 modelTransform = transform.getWorldTransformation() * mesh->getRestTransform();
			graphics->model = modelTransform;

			AABB& aabb = mesh->getAABB();
			aabb.transform(modelTransform);

			if (!aabb.isOnFrustum(*graphics->frustum))
			{
				continue;
			}

			graphics->shader->setModelMatrix(graphics->model);

			//DebugHelper::getInstance().drawAABB(aabb);
			RenderData::ObjectData objData{};
			objData.model = modelTransform;
			objData.materialIndex = mesh->getMaterialIndex();
			m_sceneBuffer.addObject(objData);

			RenderData::DrawCommand drawCommand{};
			drawCommand.vertexCount = 6;
			drawCommand.instanceCount = 1;
			drawCommand.firstIndex = 0;
			drawCommand.baseVertex = 0;
			drawCommand.baseInstance = 0;
			addDrawCommand(drawCommand);
		}

	};

	auto mat = BuiltInAssets::getByName<MaterialAsset>(SGE_MATERIAL_DEFAULT).resource();

	int slot = 6;

	// Set samplers
	for (const auto& [name, sampler] : mat->getAllSamplers())
	{
		mat->useSamplerInShader(name, sampler, graphics->shader, slot);
		slot++;
	}

	// Set uniforms
	for (const auto& [name, value] : mat->getAllUniforms())
	{
		graphics->shader->setUniformValue(name, value);
	}

	m_sceneBuffer.upload();

	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_indirectBuffer);
	glBufferSubData(GL_DRAW_INDIRECT_BUFFER, 0,
		m_drawCommands.size() * sizeof(RenderData::DrawCommand),
		m_drawCommands.data());

	auto vao = BuiltInAssets::getByName<ModelAsset>(SGE_MESH_QUAD).resource()->getPrimaryMesh()->getVAO();

	vao.Bind();
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_indirectBuffer);
	glMultiDrawElementsIndirect(
		GL_TRIANGLES,           // primitive type
		GL_UNSIGNED_INT,        // index type
		0,                      // offset into indirect buffer (0 = start)
		m_drawCommands.size(),  // how many draw commands
		0                       // stride (0 = tightly packed)
	);

	m_drawCommands.clear();
	m_sceneBuffer.clear();

	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Light pass");

	if (graphics->renderMode == RenderMode::WIREFRAME)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glDisable(GL_POLYGON_OFFSET_LINE);
	}

	// unbind gBuffer
	m_gBuffer.unbind();


	//glDisable(GL_DEPTH_TEST);

	// bind textures
	// Todo solve slots issue
	ShaderResourceRef lightPassShaderResource = BuiltInResources::get<Shader>(SGE_RESOURCE_SHADER_DEFFERED_PBR_LIGHT);
	lightPassShaderResource->use();
	lightPassShaderResource->setTextureInShader(m_positionTexture, "gPosition", 0);
	lightPassShaderResource->setTextureInShader(m_normalTexture, "gNormal", 1);
	lightPassShaderResource->setTextureInShader(m_albedoTexture, "gAlbedo", 2);
	lightPassShaderResource->setTextureInShader(m_MRATexture, "gMRA", 3);
	lightPassShaderResource->setTextureInShader(graphics->irradianceMap, "gIrradianceMap", 4);
	lightPassShaderResource->setTextureInShader(graphics->prefilterEnvMap, "gPrefilterEnvMap", 5);
	lightPassShaderResource->setTextureInShader(graphics->brdfLUT, "gBRDFIntegrationLUT", 6);
	lightPassShaderResource->setTextureInShader(graphics->shadowMap, "gShadowMap", 7);
	lightPassShaderResource->setTextureInShader(m_TangentTexture, "gTangnet", 9);
	lightPassShaderResource->bindUniformBlockToBindPoint("Time", 0);
	lightPassShaderResource->bindUniformBlockToBindPoint("Lights", 1);
	lightPassShaderResource->setUniformValue("cameraPos", graphics->cameraPos);
	lightPassShaderResource->setUniformValue("lightSpaceMatrix", graphics->lightSpaceMatrix);
	lightPassShaderResource->setUniformValue("useSSAO", graphics->useSSAO);

	graphics->renderView->bind();

	{
		// render to quad
		auto vao = m_quad->getPrimaryMesh()->getVAO();
		RenderCommand::draw(vao);
	}

	glPopDebugGroup();

	//graphics->renderView->unbind();
}

const FrameBufferObject& IndirectRenderer::getGBuffer() const
{
	return m_gBuffer;
}

void IndirectRenderer::beginFrame()
{
}

void IndirectRenderer::endFrame()
{

	//m_sceneBuffer.upload();

	//glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_indirectBuffer);
	//glBufferSubData(GL_DRAW_INDIRECT_BUFFER, 0,
	//	m_drawCommands.size() * sizeof(DrawCommand),
	//	m_drawCommands.data());
}

void IndirectRenderer::resize(int w, int h)
{
	setupGBuffer(w, h);
}

//void DeferredRenderer::reloadShaders()
//{
//	m_gBufferShader = Shader::load(SGE_ROOT_DIR "Resources/Engine/Shaders/PBR_GeomPassShader.glsl"); // TODO fix, now when its a built in asset it will cause issues
//	m_lightPassShader = Shader::load(SGE_ROOT_DIR "Resources/Engine/Shaders/PBR_LightPassShader.glsl");
//	m_ssaoPassShader = Shader::load(SGE_ROOT_DIR "Resources/Engine/Shaders/SSAOPassShader.glsl");
//}
