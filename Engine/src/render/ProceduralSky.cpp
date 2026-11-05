#include "render/ProceduralSky.h"

#include "runtime/SceneEnvironment.h"
#include "texture/Texture.h"
#include "render/FrameBufferObject.h"
#include "render/RenderBufferObject.h"
#include "render/Shader.h"
#include "render/RenderCommand.h"
#include "render/IBL.h"
#include "geometry/Model.h"
#include "memory/BuiltInAssets.h"
#include "core/Logger.h"

#include "GL/glew.h"
#include "glm/ext.hpp"

namespace
{
	constexpr int SKY_SIZE = 256;

	TextureResourceRef renderSkyCubemap()
	{
		auto skyShader = Shader::load(SGE_ROOT_DIR "Resources/Engine/Shaders/ProceduralSkyShader.glsl");

		FrameBufferObject fbo;
		fbo.bind();

		TextureData textureData;
		textureData.target = TextureTarget::TEXTURE_CUBE_MAP;
		textureData.width = SKY_SIZE;
		textureData.height = SKY_SIZE;
		textureData.channels = 3;
		textureData.internalFormat = TextureInternalFormat::RGB16F;
		textureData.format = TextureFormat::RGB;
		textureData.type = TextureType::FLOAT;
		textureData.filter = TextureFilter::Linear;
		textureData.wrap = TextureWrap::Clamp;
		textureData.genMipMap = true;
		auto cubemap = Texture::createTexture(textureData);

		RenderBufferObject rbo{ SKY_SIZE, SKY_SIZE };
		fbo.attachRenderBuffer(rbo.GetID(), FrameBufferObject::AttachmentType::Depth);

		if (!fbo.isComplete())
		{
			logError("Procedural sky: FBO is not complete");
			fbo.unbind();
			return TextureResourceRef::empty;
		}

		// Same face order and orientation as IBL.cpp
		glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
		glm::mat4 captureViews[] =
		{
			glm::lookAt(glm::vec3(0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			glm::lookAt(glm::vec3(0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
			glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
			glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
		};

		glViewport(0, 0, SKY_SIZE, SKY_SIZE);

		skyShader->use();
		skyShader->setProjectionMatrix(captureProjection);

		auto box = BuiltInAssets::getByName<ModelAsset>(SGE_MESH_BOX);
		auto vao = box.resource()->getPrimaryMesh()->getVAO();

		for (int i = 0; i < 6; i++)
		{
			skyShader->setViewMatrix(captureViews[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, cubemap.get()->getID(), 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			RenderCommand::draw(vao);
		}

		fbo.unbind();

		// The prefilter pass samples the mips
		cubemap->generateMipMaps();
		cubemap->unbind();

		return cubemap;
	}
}

bool ProceduralSky::init()
{
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Procedural sky");

	m_sky = renderSkyCubemap();
	if (!m_sky.isEmpty())
	{
		m_irradianceMap = IBL::generateIrradianceMap(m_sky);
		m_prefilterEnvMap = IBL::generatePrefilterEnvMap(m_sky);
	}

	glPopDebugGroup();

	return !m_sky.isEmpty() && !m_irradianceMap.isEmpty() && !m_prefilterEnvMap.isEmpty();
}

void ProceduralSky::fill(SceneEnvironment& environment) const
{
	environment.sky = m_sky;
	environment.irradianceMap = m_irradianceMap;
	environment.prefilterEnvMap = m_prefilterEnvMap;
}
