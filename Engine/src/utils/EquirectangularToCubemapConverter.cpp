#include "utils/EquirectangularToCubemapConverter.h"

#include "texture/Texture.h"
#include "render/FrameBufferObject.h"
#include "render/RenderBufferObject.h"
#include "memory/ResourceWrapper.h"
#include "render/Shader.h"

#include "GL/glew.h"
#include "glm/ext.hpp"
#include "core/Logger.h"

#include "render/RenderCommand.h"
#include "component/Component.h"
#include "geometry/MeshGroup.h"
#include "memory/BuiltInAssets.h"



ResourceWrapper<Texture> EquirectangularToCubemapConverter::fromEquirectangularToCubemap(ResourceWrapper<Texture> equirectangularTexture)
{
	//equirectangularTexture = TextureTransformer::flipVertical(equirectangularTexture);

	auto equirectangularShader = Shader::load(SGE_ROOT_DIR "Resources/Engine/Shaders/EquirectangularToCubemap.glsl");

	// Generate FBO 
	FrameBufferObject fbo;

	fbo.bind();

	// Generate cubemap
	TextureData textureData;
	textureData.target = TextureTarget::TEXTURE_CUBE_MAP;
	textureData.width = 512;
	textureData.height = 512;
	textureData.channels = 3;
	textureData.internalFormat = TextureInternalFormat::RGB16F;
	textureData.format = TextureFormat::RGB;
	textureData.type = TextureType::FLOAT;
	textureData.filter = TextureFilter::Linear;
	textureData.wrap = TextureWrap::Clamp;
	textureData.genMipMap = true;
	textureData.data = nullptr;
	auto cubemap = Texture::createTexture(textureData);

	RenderBufferObject rbo{ 512, 512 };
	fbo.attachRenderBuffer(rbo.GetID(), FrameBufferObject::AttachmentType::Depth);

	if (!fbo.isComplete())
	{
		logError("FBO is not complete!");
		return nullptr;
	}

	// Generate views and projection
	glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	glm::mat4 captureViews[] =
	{
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
	};

	// set viewport
	glViewport(0, 0, 512, 512);

	equirectangularShader->use();
	equirectangularShader->setProjectionMatrix(captureProjection);
	equirectangularShader->setUniformValue("equirectangularMap", 0);


	
	equirectangularTexture.get()->setSlot(0);
	equirectangularTexture.get()->bind();
	

	auto box = BuiltInAssets::getByName<MeshGroupAsset>(SGE_MESH_BOX);
	auto vao = box.resource()->getPrimaryMesh()->getVAO();

	// render to cube
	// Attach cube map to frame buffer
	for (int i = 0; i < 6; i++)
	{
		// set view
		equirectangularShader->setViewMatrix(captureViews[i]);

		// attach cubemap face to fbo
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, cubemap.get()->getID(), 0);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// render cube
		RenderCommand::draw(vao);
	}

	equirectangularTexture.get()->unbind();
	fbo.unbind();

	return cubemap;
}

ResourceWrapper<Texture> EquirectangularToCubemapConverter::fromCubemapToEquirectangular(ResourceWrapper<Texture> cubemapTexture)
{
	auto cubemapToEquirectangularShader = Shader::load(SGE_ROOT_DIR "Resources/Engine/Shaders/CubemapToEquirectangular.glsl");

	// Generate FBO 
	FrameBufferObject fbo;

	fbo.bind();

	int outputWidth = cubemapTexture.get()->getWidth() * 3;
	int outputHeight = cubemapTexture.get()->getHeight() * 2;

	// Generate cubemap
	TextureData textureData;
	textureData.target = TextureTarget::TEXTURE_2D;
	textureData.width = outputWidth;
	textureData.height = outputHeight;
	textureData.channels = 3;
	textureData.internalFormat = TextureInternalFormat::RGB;
	textureData.format = TextureFormat::RGB;
	textureData.type = TextureType::UNSIGNED_BYTE;
	textureData.filter = TextureFilter::Linear;
	textureData.wrap = TextureWrap::Clamp;
	textureData.data = nullptr;
	auto equirectnagular = Texture::createTexture(textureData);

	RenderBufferObject rbo{ outputWidth, outputHeight };
	fbo.attachRenderBuffer(rbo.GetID(), FrameBufferObject::AttachmentType::Depth);

	if (!fbo.isComplete())
	{
		logError("FBO is not complete!");
		return nullptr;
	}

	fbo.attachTexture(equirectnagular.get()->getID());

	// set viewport
	glViewport(0, 0, outputWidth, outputHeight);

	cubemapToEquirectangularShader->use();
	cubemapToEquirectangularShader->setUniformValue("cubemap", 0);

	cubemapTexture.get()->setSlot(0);
	cubemapTexture.get()->bind();

	auto quad = BuiltInAssets::getByName<MeshGroupAsset>(SGE_MESH_QUAD);
	auto vao = quad.resource()->getPrimaryMesh()->getVAO();

	// render to quad
	// attach cubemap face to fbo
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// render cube
	RenderCommand::draw(vao);

	fbo.unbind();

	return equirectnagular;
}
