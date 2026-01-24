#include "render/DeferredRenderer.h"

#include "memory/ResourceWrapper.h"
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
#include "runtime/Context.h"
#include "animation/Animator.h"
#include "geometry/MeshGroup.h"
#include "render/Graphics.h"
#include "core/System.h"
#include "geometry/ShapeFactory.h"
#include "render/RenderView.h"
#include "utils/DebugHelper.h"
#include "component/MeshRendererComponent.h"

#include "component/ShaderComponent.h"
#include "component/ObjectComponent.h"
#include "component/RenderableComponent.h"
#include "memory/BuiltInAssets.h"
#include "memory/BuiltInResources.h"

static float lerp(float a, float b, float t)
{
	return a + t * (b - a);
}

bool DeferredRenderer::setupGBuffer(int width, int height)
{
	m_gBuffer.bind();

	// Generate Texture for Position data
	m_positionTexture = Texture::createTexture(width, height, 3, Texture::InternalFormat::RGBA16F, Texture::Format::RGBA, Texture::Type::FLOAT);
	m_gBuffer.attachTexture(m_positionTexture.get()->getID(), GL_COLOR_ATTACHMENT0);

	// Generate Texture for Normal data
	m_normalTexture = Texture::createTexture(width, height, 3, Texture::InternalFormat::RGBA16F, Texture::Format::RGBA, Texture::Type::FLOAT);
	m_gBuffer.attachTexture(m_normalTexture.get()->getID(), GL_COLOR_ATTACHMENT1);

	// Generate Texture for Albedo
	m_albedoTexture = Texture::createTexture(width, height, 3, Texture::InternalFormat::RGBA, Texture::Format::RGBA, Texture::Type::UNSIGNED_BYTE);
	m_gBuffer.attachTexture(m_albedoTexture.get()->getID(), GL_COLOR_ATTACHMENT2);

	// Generate Texture for MRA
	m_MRATexture = Texture::createTexture(width, height, 3, Texture::InternalFormat::RGBA, Texture::Format::RGBA, Texture::Type::UNSIGNED_BYTE);
	m_gBuffer.attachTexture(m_MRATexture.get()->getID(), GL_COLOR_ATTACHMENT3);

	// Generate Texture for Position ViewSpace data
	m_positionTextureVS = Texture::createTexture(width, height, 3, Texture::InternalFormat::RGBA16F, Texture::Format::RGBA, Texture::Type::FLOAT);
	m_gBuffer.attachTexture(m_positionTextureVS.get()->getID(), GL_COLOR_ATTACHMENT4);

	// Generate Texture for Normal ViewSpace data
	m_normalTextureVS = Texture::createTexture(width, height, 3, Texture::InternalFormat::RGBA16F, Texture::Format::RGBA, Texture::Type::FLOAT);
	m_gBuffer.attachTexture(m_normalTextureVS.get()->getID(), GL_COLOR_ATTACHMENT5);

	// Generate Texture for Tangent data
	m_TangentTexture = Texture::createTexture(width, height, 3, Texture::InternalFormat::RGBA16F, Texture::Format::RGBA, Texture::Type::FLOAT);
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

bool DeferredRenderer::setupSSAO(int width, int height)
{
	int ssaoBufferWidth = width * .5f;
	int ssaoBufferHeight = height * .5f;

	// Generate SSAO kernel
	m_ssaoKernel.reserve(64);
	auto rand = Engine::get()->getRandomSystem();

	for (int i = 0; i < 64; i++)
	{
		glm::vec3 sample(rand->rand() * 2.0	- 1.0, rand->rand() * 2.0 - 1.0, rand->rand());
		sample = glm::normalize(sample);
		sample *= rand->rand();
		float scale = (float)i / 64;
		scale = lerp(0.1f, 1.0f, scale * scale);
		sample *= scale;
		m_ssaoKernel.push_back(sample);
	}

	// Generate SSAO Noise
	std::vector<glm::vec3> ssaoNoise;
	ssaoNoise.reserve(16);
	for (int i = 0; i < 16; i++)
	{
		ssaoNoise.push_back({
				rand->rand() * 2.0 - 1.0,
				rand->rand() * 2.0 - 1.0,
				0.f
			});
	}

	m_ssaoNoiseTexture = Texture::createTexture(4, 4, 1,
		Texture::InternalFormat::RGBA32F,
		Texture::Format::RGB,
		Texture::Type::FLOAT, 
		Texture::TextureFilter::Nearest,
		Texture::TextureWrap::Repeat,
		&ssaoNoise[0]
		);

	// Initialize SSAO FBO
	m_ssaoFBO.bind();

	m_ssaoColorBuffer = Texture::createTexture(ssaoBufferWidth, ssaoBufferHeight, 3, Texture::InternalFormat::R16, Texture::Format::RED, Texture::Type::FLOAT);
	m_ssaoFBO.attachTexture(m_ssaoColorBuffer.get()->getID(), GL_COLOR_ATTACHMENT0);

	unsigned int attachments[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, attachments);

	// Create RBO and attach to FBO
	m_ssaoRenderBuffer = RenderBufferObject(ssaoBufferWidth, ssaoBufferHeight);
	m_ssaoFBO.attachRenderBuffer(m_ssaoRenderBuffer.GetID(), FrameBufferObject::AttachmentType::Depth_Stencil);

	if (!m_ssaoFBO.isComplete())
	{
		logError("FBO is not complete!");
		return false;
	}

	m_ssaoFBO.unbind();

	m_ssaoPassShader = Shader::load(SGE_ROOT_DIR "Resources/Engine/Shaders/SSAOPassShader.glsl");

	// Initialize SSAO Blur
	m_ssaoBlurFBO.bind();

	m_ssaoBlurColorBuffer = Texture::createTexture(ssaoBufferWidth, ssaoBufferHeight, 3, Texture::InternalFormat::R16, Texture::Format::RED, Texture::Type::FLOAT);
	m_ssaoBlurFBO.attachTexture(m_ssaoBlurColorBuffer.get()->getID(), GL_COLOR_ATTACHMENT0);

	// Create RBO and attach to FBO
	m_ssaoBlurRenderBuffer = RenderBufferObject(ssaoBufferWidth, ssaoBufferHeight);
	m_ssaoBlurFBO.attachRenderBuffer(m_ssaoBlurRenderBuffer.GetID(), FrameBufferObject::AttachmentType::Depth_Stencil);

	if (!m_ssaoBlurFBO.isComplete())
	{
		logError("FBO is not complete!");
		return false;
	}

	m_ssaoBlurFBO.unbind();

	m_ssaoBlurPassShader = Shader::load(SGE_ROOT_DIR "Resources/Engine/Shaders/SSAOBlurPassShader.glsl");

	DebugHelper::getInstance().registerTextureForDebug("SSAO Color", m_ssaoBlurColorBuffer);

	return true;
}

bool DeferredRenderer::init()
{

	auto width = Engine::get()->getWindow()->getWidth();
	auto height = Engine::get()->getWindow()->getHeight();

	setupGBuffer(width, height);

	setupSSAO(width, height);

	// Generate screen quad
	m_quad = Engine::get()->getSubSystem<Assets>()->getAssetFromName(SGE_MESH_QUAD).as<MeshGroupAsset>().resource();

	return true;
}

void DeferredRenderer::render()
{
	auto graphics = Engine::get()->getSubSystem<Graphics>();

    graphics->shader->setModelMatrix(graphics->model);
    graphics->shader->setViewMatrix(graphics->view);
    graphics->shader->setProjectionMatrix(graphics->projection);
	graphics->shader->bindUniformBlockToBindPoint("Time", 0);
	graphics->shader->bindUniformBlockToBindPoint("Lights", 1);

	graphics->material->use();

	// Draw
	auto instanceBatch = graphics->entity.tryGetComponent<InstanceBatch>();
	if (!instanceBatch)
	{
		graphics->shader->setUniformValue("isGpuInstanced", false);
		RenderCommand::draw(graphics->mesh->getVAO());
	}
	else
	{
		graphics->shader->setUniformValue("isGpuInstanced", true);
		RenderCommand::drawInstanced(graphics->mesh->getVAO(), instanceBatch->getCount());
	}
}

void DeferredRenderer::renderScene(Scene* scene)
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

	graphics->shader = BuiltInResources::get<Shader>(SGE_RESOURCE_SHADER_DEFFERED_PBR_GEOM);
	graphics->shader->use();

	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "G-Buffer pass");

	// Render all objects
	for (auto&& [entity, meshRenderer, transform, obj] :
		scene->getRegistry().getRegistry().view<MeshRendererComponent, Transformation, ObjectComponent>().each())
	{
		if (meshRenderer.renderTechnique != MeshRendererComponent::RenderTechnique::Deferred)
			continue;

		Entity entityHandler{ entity, &scene->getRegistry() };

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
			if (graphics->material->getMaterialRenderMode() != MaterialRenderMode::Opaque)
			{
				continue;
			}

			std::string captionSubmeshGPU = "About to render submesh: '" + mesh->getName() + "' using material: '" + graphics->material->getName() + "'";
			glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, captionSubmeshGPU.c_str());

			// draw model
			render();

			glPopDebugGroup();
		}

		glPopDebugGroup();
		
	};

	glPopDebugGroup();

	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Light pass");

	if (graphics->renderMode == RenderMode::WIREFRAME)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glDisable(GL_POLYGON_OFFSET_LINE);
	}

	// unbind gBuffer
	m_gBuffer.unbind();


	//glDisable(GL_DEPTH_TEST);
	
	if (graphics->useSSAO)
	{
		glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "SSAO pass");

		glDisable(GL_DEPTH_TEST);

		m_ssaoFBO.bind();
		m_ssaoPassShader->use();
		glClear(GL_COLOR_BUFFER_BIT);

		// SSAO
		m_ssaoPassShader->setTextureInShader(m_positionTextureVS, "gPositionVS", 0);
		m_ssaoPassShader->setTextureInShader(m_normalTextureVS, "gNormalVS", 1);
		m_ssaoPassShader->setTextureInShader(m_ssaoNoiseTexture, "gSSAONoise", 2);

		// TODO remove
		auto width = Engine::get()->getWindow()->getWidth();
		auto height = Engine::get()->getWindow()->getHeight();

		// We set the viewport to half the screen size to improve the SSAO performance
		RenderCommand::setViewport(0, 0, width / 2.f, height / 2.f);

		m_ssaoPassShader->setUniformValue("screenWidth", (int)(width / 2.f));
		m_ssaoPassShader->setUniformValue("screenHeight", (int)(height / 2.f));

		for (unsigned int i = 0; i < 64; ++i)
		{
			m_ssaoPassShader->setUniformValue("ssaoKernel[" + std::to_string(i) + "]", m_ssaoKernel[i]);
		}

		m_ssaoPassShader->setUniformValue("view", graphics->view);
		m_ssaoPassShader->setUniformValue("projection", graphics->projection);

		{
			// render to quad
			auto vao = m_quad->getPrimaryMesh()->getVAO();
			RenderCommand::draw(vao);
		}

		//glClear(GL_COLOR_BUFFER_BIT);

		m_ssaoBlurFBO.bind();
		m_ssaoBlurPassShader->use();

		m_ssaoBlurPassShader->setTextureInShader(m_ssaoColorBuffer, "gSSAOColorBuffer", 0);


		{
			// render to quad
			auto vao = m_quad->getPrimaryMesh()->getVAO();
			RenderCommand::draw(vao);
		}

		glEnable(GL_DEPTH_TEST);

		// We set the viewport back to original size
		RenderCommand::setViewport(0, 0, width, height);

		glPopDebugGroup();
	}

	// bind textures
	// Todo solve slots issue
	ResourceWrapper<Shader> lightPassShaderResource = BuiltInResources::get<Shader>(SGE_RESOURCE_SHADER_DEFFERED_PBR_LIGHT);
	lightPassShaderResource->use();
	lightPassShaderResource->setTextureInShader(m_positionTexture, "gPosition", 0);
	lightPassShaderResource->setTextureInShader(m_normalTexture, "gNormal", 1);
	lightPassShaderResource->setTextureInShader(m_albedoTexture, "gAlbedo", 2);
	lightPassShaderResource->setTextureInShader(m_MRATexture, "gMRA", 3);
	lightPassShaderResource->setTextureInShader(graphics->irradianceMap, "gIrradianceMap", 4);
	lightPassShaderResource->setTextureInShader(graphics->prefilterEnvMap, "gPrefilterEnvMap", 5);
	lightPassShaderResource->setTextureInShader(graphics->brdfLUT, "gBRDFIntegrationLUT", 6);
	lightPassShaderResource->setTextureInShader(graphics->shadowMap, "gShadowMap", 7);
	lightPassShaderResource->setTextureInShader(m_ssaoBlurColorBuffer, "gSSAOColorBuffer", 8);
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

const FrameBufferObject& DeferredRenderer::getGBuffer() const
{
	return m_gBuffer;
}

void DeferredRenderer::resize(int w, int h)
{
	setupGBuffer(w, h);
	setupSSAO(w, h);
}

//void DeferredRenderer::reloadShaders()
//{
//	m_gBufferShader = Shader::load(SGE_ROOT_DIR "Resources/Engine/Shaders/PBR_GeomPassShader.glsl"); // TODO fix, now when its a built in asset it will cause issues
//	m_lightPassShader = Shader::load(SGE_ROOT_DIR "Resources/Engine/Shaders/PBR_LightPassShader.glsl");
//	m_ssaoPassShader = Shader::load(SGE_ROOT_DIR "Resources/Engine/Shaders/SSAOPassShader.glsl");
//}
