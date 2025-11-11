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
#include "geometry/MeshCollection.h"
#include "render/Graphics.h"
#include "core/System.h"
#include "geometry/ShapeFactory.h"
#include "render/RenderView.h"
#include "utils/DebugHelper.h"
#include "component/MeshComponent.h"
#include "component/MaterialComponent.h"
#include "component/ShaderComponent.h"
#include "component/ObjectComponent.h"
#include "component/RenderableComponent.h"
#include "memory/BuiltInAssets.h"

static float lerp(float a, float b, float t)
{
	return a + t * (b - a);
}

bool DeferredRenderer::setupGBuffer(int width, int height)
{
	m_gBuffer.bind();

	// Generate Texture for Position data
	m_positionTexture = Texture::createEmptyTexture(width, height, GL_RGBA16F, GL_RGBA, GL_FLOAT);
	m_gBuffer.attachTexture(m_positionTexture.get()->getID(), GL_COLOR_ATTACHMENT0);

	// Generate Texture for Normal data
	m_normalTexture = Texture::createEmptyTexture(width, height, GL_RGBA16F, GL_RGBA, GL_FLOAT);
	m_gBuffer.attachTexture(m_normalTexture.get()->getID(), GL_COLOR_ATTACHMENT1);

	// Generate Texture for Albedo
	m_albedoTexture = Texture::createEmptyTexture(width, height, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE);
	m_gBuffer.attachTexture(m_albedoTexture.get()->getID(), GL_COLOR_ATTACHMENT2);

	// Generate Texture for MRA
	m_MRATexture = Texture::createEmptyTexture(width, height, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE);
	m_gBuffer.attachTexture(m_MRATexture.get()->getID(), GL_COLOR_ATTACHMENT3);

	// Generate Texture for Position ViewSpace data
	m_positionTextureVS = Texture::createEmptyTexture(width, height, GL_RGBA16F, GL_RGBA, GL_FLOAT);
	m_gBuffer.attachTexture(m_positionTextureVS.get()->getID(), GL_COLOR_ATTACHMENT4);

	// Generate Texture for Normal ViewSpace data
	m_normalTextureVS = Texture::createEmptyTexture(width, height, GL_RGBA16F, GL_RGBA, GL_FLOAT);
	m_gBuffer.attachTexture(m_normalTextureVS.get()->getID(), GL_COLOR_ATTACHMENT5);

	unsigned int attachments[6] = { 
		GL_COLOR_ATTACHMENT0, 
		GL_COLOR_ATTACHMENT1, 
		GL_COLOR_ATTACHMENT2, 
		GL_COLOR_ATTACHMENT3, 
		GL_COLOR_ATTACHMENT4, 
		GL_COLOR_ATTACHMENT5 };
	glDrawBuffers(6, attachments);

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

	m_ssaoNoiseTexture = Texture::create2DTextureFromBuffer(4, 4,
		GL_RGBA32F,
		GL_RGB,
		GL_FLOAT, {
		{ GL_TEXTURE_MIN_FILTER,	GL_NEAREST	},
		{ GL_TEXTURE_MAG_FILTER,	GL_NEAREST	},
		{ GL_TEXTURE_WRAP_S,		GL_REPEAT	},
		{ GL_TEXTURE_WRAP_T,		GL_REPEAT   } },
		true,
		&ssaoNoise[0]
		);

	// Initialize SSAO FBO
	m_ssaoFBO.bind();

	m_ssaoColorBuffer = Texture::createEmptyTexture(ssaoBufferWidth, ssaoBufferHeight, GL_RED, GL_RED, GL_FLOAT);
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

	m_ssaoPassShader = Shader::load(SGE_ROOT_DIR + "Resources/Engine/Shaders/SSAOPassShader.glsl");

	// Initialize SSAO Blur
	m_ssaoBlurFBO.bind();

	m_ssaoBlurColorBuffer = Texture::createEmptyTexture(ssaoBufferWidth, ssaoBufferHeight, GL_RED, GL_RED, GL_FLOAT);
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

	m_ssaoBlurPassShader = Shader::load(SGE_ROOT_DIR + "Resources/Engine/Shaders/SSAOBlurPassShader.glsl");

	DebugHelper::getInstance().registerTextureForDebug("SSAO Color", m_ssaoBlurColorBuffer);

	return true;
}

bool DeferredRenderer::init()
{
	m_gBufferShader = Shader::load(SGE_ROOT_DIR + "Resources/Engine/Shaders/PBR_GeomPassShader.glsl");
	m_lightPassShader = Shader::load(SGE_ROOT_DIR + "Resources/Engine/Shaders/PBR_LightPassShader.glsl");

	auto width = Engine::get()->getWindow()->getWidth();
	auto height = Engine::get()->getWindow()->getHeight();

	setupGBuffer(width, height);

	setupSSAO(width, height);

	// Generate screen quad
	UUID quadUUID = Engine::get()->getSubSystem<Assets>()->getAssetFromName(SGE_MESH_QUAD);
	m_quad = Engine::get()->getSubSystem<Assets>()->getAsset(quadUUID).resource.as<MeshCollection>();

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

	graphics->material->use(graphics->shader);

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

	graphics->entityGroup.clear();
	for (auto&& [entity, mesh, transform, renderable] :
		scene->getRegistry().getRegistry().view<MeshComponent, Transformation, RenderableComponent>(entt::exclude<ShaderComponent>).each())
	{
		if (renderable.renderTechnique == RenderableComponent::RenderTechnique::Deferred)
		{
			Entity entityhandler{ entity, &scene->getRegistry() };
			graphics->entityGroup.push_back(entityhandler);
		}
	}
	
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

	graphics->shader = m_gBufferShader;
	graphics->shader->use();

	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "G-Buffer pass");

	// Render all objects
	for (auto& entityHandler : graphics->entityGroup)
	{
		std::string name = entityHandler.getComponent<ObjectComponent>().name;
		logTrace("About to render {}", name);

		ResourceWrapper<MeshCollection> meshCollecton = entityHandler.getComponent<MeshComponent>().mesh.resource();

		auto animator = entityHandler.tryGetComponent<Animator>();
		if (!animator || animator->m_currentAnimation.isEmpty())
		{
			graphics->shader->setUniformValue("isAnimated", false);
		}
		else
		{
			std::vector<glm::mat4> finalBoneMatrices;
			animator->getFinalBoneMatrices(meshCollecton.get(), finalBoneMatrices);
			for (int i = 0; i < finalBoneMatrices.size(); ++i)
			{
				graphics->shader->setUniformValue("finalBonesMatrices[" + std::to_string(i) + "]", finalBoneMatrices[i]);
			}

			graphics->shader->setUniformValue("isAnimated", true);
		}

		for (auto mesh : meshCollecton.get()->getMeshes())
		{
			if (!prepareMeshForRender(mesh.get(), entityHandler))
			{
				continue;
			}

			// Only render Opaque objects
			if (!graphics->material->isOpaque())
			{
				continue;
			}

			// draw model
			render();
		}
		
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

		m_ssaoPassShader->setUniformValue("screenWidth", width / 2.f);
		m_ssaoPassShader->setUniformValue("screenHeight", height / 2.f);

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
	m_lightPassShader->setTextureInShader(m_positionTexture, "gPosition", 0);
	m_lightPassShader->setTextureInShader(m_normalTexture, "gNormal", 1);
	m_lightPassShader->setTextureInShader(m_albedoTexture, "gAlbedo", 2);
	m_lightPassShader->setTextureInShader(m_MRATexture, "gMRA", 3);
	m_lightPassShader->setTextureInShader(graphics->irradianceMap, "gIrradianceMap", 4);
	m_lightPassShader->setTextureInShader(graphics->prefilterEnvMap, "gPrefilterEnvMap", 5);
	m_lightPassShader->setTextureInShader(graphics->brdfLUT, "gBRDFIntegrationLUT", 6);
	m_lightPassShader->setTextureInShader(graphics->shadowMap, "gShadowMap", 7);
	m_lightPassShader->setTextureInShader(m_ssaoBlurColorBuffer, "gSSAOColorBuffer", 8);

	graphics->renderView->bind();

	// bind fShader
	m_lightPassShader->use();

	m_lightPassShader->bindUniformBlockToBindPoint("Time", 0);
	m_lightPassShader->bindUniformBlockToBindPoint("Lights", 1);

	m_lightPassShader->setUniformValue("cameraPos", graphics->cameraPos);
	m_lightPassShader->setUniformValue("lightSpaceMatrix", graphics->lightSpaceMatrix);

	m_lightPassShader->setUniformValue("useSSAO", graphics->useSSAO);

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

void DeferredRenderer::reloadShaders()
{
	m_gBufferShader = Shader::load(SGE_ROOT_DIR + "Resources/Engine/Shaders/PBR_GeomPassShader.glsl");
	m_lightPassShader = Shader::load(SGE_ROOT_DIR + "Resources/Engine/Shaders/PBR_LightPassShader.glsl");
	m_ssaoPassShader = Shader::load(SGE_ROOT_DIR + "Resources/Engine/Shaders/SSAOPassShader.glsl");
}
