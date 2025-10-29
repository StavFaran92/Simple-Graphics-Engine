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
#include "render/Renderer2D.h"
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

static float lerp(float a, float b, float t)
{
	return a + t * (b - a);
}

DeferredRenderer::DeferredRenderer(Scene* scene) 
	: m_scene(scene)
{
}

bool DeferredRenderer::setupGBuffer()
{
	m_gBuffer.bind();

	auto width = Engine::get()->getWindow()->getWidth();
	auto height = Engine::get()->getWindow()->getHeight();

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

	unsigned int attachments[6] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5 };
	glDrawBuffers(6, attachments);

	// Create RBO and attach to FBO
	m_gBuffer.attachRenderBuffer(m_renderBuffer.GetID(), FrameBufferObject::AttachmentType::Depth_Stencil);

	if (!m_gBuffer.isComplete())
	{
		logError("FBO is not complete!");
		return false;
	}

	m_gBuffer.unbind();

	m_gBufferShader = Shader::load(SGE_ROOT_DIR + "Resources/Engine/Shaders/PBR_GeomPassShader.glsl");
	m_lightPassShader = Shader::load(SGE_ROOT_DIR + "Resources/Engine/Shaders/PBR_LightPassShader.glsl");

	return true;
}

bool DeferredRenderer::setupSSAO()
{
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

	auto width = Engine::get()->getWindow()->getWidth();
	auto height = Engine::get()->getWindow()->getHeight();

	// Initialize SSAO FBO
	m_ssaoFBO.bind();

	m_ssaoColorBuffer = Texture::createEmptyTexture(width, height, GL_RED, GL_RED, GL_FLOAT);
	m_ssaoFBO.attachTexture(m_ssaoColorBuffer.get()->getID(), GL_COLOR_ATTACHMENT0);

	unsigned int attachments[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, attachments);

	// Create RBO and attach to FBO
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

	m_ssaoBlurColorBuffer = Texture::createEmptyTexture(width, height, GL_RED, GL_RED, GL_FLOAT);
	m_ssaoBlurFBO.attachTexture(m_ssaoBlurColorBuffer.get()->getID(), GL_COLOR_ATTACHMENT0);

	// Create RBO and attach to FBO
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
	setupGBuffer();

	setupSSAO();

	// Generate screen quad
	m_quad = ShapeFactory::createQuad(&Engine::get()->getContext()->getRegistry());
	m_quad.RemoveComponent<RenderableComponent>();
	m_quad.RemoveComponent<ObjectComponent>();

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

    if (graphics->material)
    {
		graphics->material->use(graphics->shader);
    }

	// Draw
	auto instanceBatch = graphics->entity->tryGetComponent<InstanceBatch>();
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
	clear();

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
			graphics->entity = &entityHandler;
			graphics->mesh = mesh.get();
			auto& transform = entityHandler.getComponent<Transformation>();
			glm::mat4 modelTransform = transform.getWorldTransformation() * mesh->getRestTransform();
			graphics->model = modelTransform;

			// TODO get this to work
			AABB& aabb = mesh.get()->getAABB();
			aabb.adjustToTransform(transform);

			if (!aabb.isOnFrustum(*graphics->frustum))
			{
				//continue; todo fix
			}

			graphics->material = Engine::get()->getDefaultMaterial().get();

			auto matIndex = mesh->getMaterialIndex();
			auto materialComponent = graphics->entity->tryGetComponent<MaterialComponent>();
			if (materialComponent)
			{

				graphics->material = materialComponent->at(matIndex).get();

				// Only render Opaque objects
				if (!graphics->material->isOpaque())
				{
					continue;
				}
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
	
#if 1
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

	m_ssaoPassShader->setUniformValue("screenWidth", width);
	m_ssaoPassShader->setUniformValue("screenHeight", height);

	for (unsigned int i = 0; i < 64; ++i)
	{
		m_ssaoPassShader->setUniformValue("ssaoKernel[" + std::to_string(i) + "]", m_ssaoKernel[i]);
	}

	m_ssaoPassShader->setUniformValue("view", graphics->view);
	m_ssaoPassShader->setUniformValue("projection", graphics->projection);

	{
		// render to quad
		auto vao = m_quad.getComponent<MeshComponent>().mesh.resource()->getPrimaryMesh()->getVAO();
		RenderCommand::draw(vao);
	}

	//glClear(GL_COLOR_BUFFER_BIT);
	
	m_ssaoBlurFBO.bind();
	m_ssaoBlurPassShader->use();

	m_ssaoBlurPassShader->setTextureInShader(m_ssaoColorBuffer, "gSSAOColorBuffer", 0);


	{
		// render to quad
		auto vao = m_quad.getComponent<MeshComponent>().mesh.resource()->getPrimaryMesh()->getVAO();
		RenderCommand::draw(vao);
	}

	glEnable(GL_DEPTH_TEST);

	glPopDebugGroup();

#endif

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
		auto& mesh = m_quad.getComponent<MeshComponent>().mesh.get()->getPrimaryMesh();
		RenderCommand::draw(mesh->getVAO());
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
	m_renderBuffer = RenderBufferObject(w, h);

	m_gBuffer.bind();

	m_positionTexture = Texture::createEmptyTexture(w, h, GL_RGBA16F, GL_RGBA, GL_FLOAT);
	m_gBuffer.attachTexture(m_positionTexture.get()->getID(), GL_COLOR_ATTACHMENT0);

	m_normalTexture = Texture::createEmptyTexture(w, h, GL_RGBA16F, GL_RGBA, GL_FLOAT);
	m_gBuffer.attachTexture(m_normalTexture.get()->getID(), GL_COLOR_ATTACHMENT1);

	m_albedoTexture = Texture::createEmptyTexture(w, h, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE);
	m_gBuffer.attachTexture(m_albedoTexture.get()->getID(), GL_COLOR_ATTACHMENT2);

	m_MRATexture = Texture::createEmptyTexture(w, h, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE);
	m_gBuffer.attachTexture(m_MRATexture.get()->getID(), GL_COLOR_ATTACHMENT3);

	unsigned int attachments[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
	glDrawBuffers(4, attachments);

	m_gBuffer.attachRenderBuffer(m_renderBuffer.GetID(), FrameBufferObject::AttachmentType::Depth_Stencil);
	m_gBuffer.unbind();

	m_ssaoRenderBuffer = RenderBufferObject(w, h);
	m_ssaoFBO.bind();
	m_ssaoColorBuffer = Texture::createEmptyTexture(w, h, GL_RED, GL_RED, GL_FLOAT);
	m_ssaoFBO.attachTexture(m_ssaoColorBuffer.get()->getID(), GL_COLOR_ATTACHMENT0);
	unsigned int ssaoAttachments[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, ssaoAttachments);
	m_ssaoFBO.attachRenderBuffer(m_ssaoRenderBuffer.GetID(), FrameBufferObject::AttachmentType::Depth_Stencil);
	m_ssaoFBO.unbind();

	m_ssaoBlurRenderBuffer = RenderBufferObject(w, h);
	m_ssaoBlurFBO.bind();
	m_ssaoBlurColorBuffer = Texture::createEmptyTexture(w, h, GL_RED, GL_RED, GL_FLOAT);
	m_ssaoBlurFBO.attachTexture(m_ssaoBlurColorBuffer.get()->getID(), GL_COLOR_ATTACHMENT0);
	m_ssaoBlurFBO.attachRenderBuffer(m_ssaoBlurRenderBuffer.GetID(), FrameBufferObject::AttachmentType::Depth_Stencil);
	m_ssaoBlurFBO.unbind();
}
