#include "systems/SSAOSystem.h"

#include "gl/glew.h"

#include "core/Engine.h"
#include "core/Window.h"
#include "core/Random.h"
#include "core/Logger.h"
#include "render/Graphics.h"
#include "render/Shader.h"
#include "render/RenderCommand.h"
#include "texture/Texture.h"
#include "memory/BuiltInAssets.h"
#include "geometry/Model.h"
#include "geometry/Mesh.h"
#include "utils/DebugHelper.h"

namespace
{
	float lerp(float a, float b, float t)
	{
		return a + t * (b - a);
	}
}

SSAOSystem::SSAOSystem()
{
	Engine::get()->registerSubSystem<SSAOSystem>(this);
}

bool SSAOSystem::init()
{
	auto width = Engine::get()->getWindow()->getWidth();
	auto height = Engine::get()->getWindow()->getHeight();

	return setup(width, height);
}

bool SSAOSystem::resize(int width, int height)
{
	return setup(width, height);
}

bool SSAOSystem::setup(int width, int height)
{
	int ssaoBufferWidth = width * .5f;
	int ssaoBufferHeight = height * .5f;

	// Generate SSAO kernel
	m_kernel.clear();
	m_kernel.reserve(64);
	auto rand = Engine::get()->getRandomSystem();

	for (int i = 0; i < 64; i++)
	{
		glm::vec3 sample(rand->rand() * 2.0 - 1.0, rand->rand() * 2.0 - 1.0, rand->rand());
		sample = glm::normalize(sample);
		sample *= rand->rand();
		float scale = (float)i / 64;
		scale = lerp(0.1f, 1.0f, scale * scale);
		sample *= scale;
		m_kernel.push_back(sample);
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

	m_noiseTexture = Texture::createTexture(4, 4, 1,
		TextureInternalFormat::RGBA32F,
		TextureFormat::RGB,
		TextureType::FLOAT,
		TextureFilter::Nearest,
		TextureWrap::Repeat,
		ImageBuffer{
			reinterpret_cast<const unsigned char*>(ssaoNoise.data()),
			ssaoNoise.size() * sizeof(glm::vec3)
		}
	);

	// Initialize SSAO FBO
	m_fbo.bind();

	m_colorBuffer = Texture::createTexture(ssaoBufferWidth, ssaoBufferHeight, 3, TextureInternalFormat::R16, TextureFormat::RED, TextureType::FLOAT);
	m_fbo.attachTexture(m_colorBuffer.get()->getID(), GL_COLOR_ATTACHMENT0);

	unsigned int attachments[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, attachments);

	m_renderBuffer = RenderBufferObject(ssaoBufferWidth, ssaoBufferHeight);
	m_fbo.attachRenderBuffer(m_renderBuffer.GetID(), FrameBufferObject::AttachmentType::Depth_Stencil);

	if (!m_fbo.isComplete())
	{
		logError("SSAO FBO is not complete!");
		return false;
	}

	m_fbo.unbind();

	m_passShader = Shader::load(SGE_ROOT_DIR "Resources/Engine/Shaders/SSAOPassShader.glsl");

	// Initialize SSAO Blur
	m_blurFBO.bind();

	m_blurColorBuffer = Texture::createTexture(ssaoBufferWidth, ssaoBufferHeight, 3, TextureInternalFormat::R16, TextureFormat::RED, TextureType::FLOAT);
	m_blurFBO.attachTexture(m_blurColorBuffer.get()->getID(), GL_COLOR_ATTACHMENT0);

	m_blurRenderBuffer = RenderBufferObject(ssaoBufferWidth, ssaoBufferHeight);
	m_blurFBO.attachRenderBuffer(m_blurRenderBuffer.GetID(), FrameBufferObject::AttachmentType::Depth_Stencil);

	if (!m_blurFBO.isComplete())
	{
		logError("SSAO Blur FBO is not complete!");
		return false;
	}

	m_blurFBO.unbind();

	m_blurPassShader = Shader::load(SGE_ROOT_DIR "Resources/Engine/Shaders/SSAOBlurPassShader.glsl");

	DebugHelper::getInstance().registerTextureForDebug("SSAO Color", m_blurColorBuffer);

	return true;
}

void SSAOSystem::draw(TextureResourceRef positionVS, TextureResourceRef normalVS)
{
	auto graphics = Engine::get()->getSubSystem<Graphics>();

	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "SSAO pass");

	glDisable(GL_DEPTH_TEST);

	m_fbo.bind();
	m_passShader->use();
	glClear(GL_COLOR_BUFFER_BIT);

	m_passShader->setTextureInShader(positionVS, "gPositionVS", 0);
	m_passShader->setTextureInShader(normalVS, "gNormalVS", 1);
	m_passShader->setTextureInShader(m_noiseTexture, "gSSAONoise", 2);

	auto width = Engine::get()->getWindow()->getWidth();
	auto height = Engine::get()->getWindow()->getHeight();

	// We set the viewport to half the screen size to improve the SSAO performance
	RenderCommand::setViewport(0, 0, width / 2.f, height / 2.f);

	m_passShader->setUniformValue("screenWidth", (int)(width / 2.f));
	m_passShader->setUniformValue("screenHeight", (int)(height / 2.f));

	for (unsigned int i = 0; i < 64; ++i)
	{
		m_passShader->setUniformValue("ssaoKernel[" + std::to_string(i) + "]", m_kernel[i]);
	}

	m_passShader->setUniformValue("view", graphics->view);
	m_passShader->setUniformValue("projection", graphics->projection);

	{
		// render to quad
		auto vao = BuiltInAssets::getByName<ModelAsset>(SGE_MESH_QUAD).resource()->getPrimaryMesh()->getVAO();
		RenderCommand::draw(vao);
	}

	m_blurFBO.bind();
	m_blurPassShader->use();

	m_blurPassShader->setTextureInShader(m_colorBuffer, "gSSAOColorBuffer", 0);

	{
		// render to quad
		auto vao = BuiltInAssets::getByName<ModelAsset>(SGE_MESH_QUAD).resource()->getPrimaryMesh()->getVAO();
		RenderCommand::draw(vao);
	}

	glEnable(GL_DEPTH_TEST);

	// We set the viewport back to original size
	RenderCommand::setViewport(0, 0, width, height);

	glPopDebugGroup();
}
