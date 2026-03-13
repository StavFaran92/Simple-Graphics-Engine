#include "render/IBL.h"

#include "texture/Texture.h"
#include "render/FrameBufferObject.h"
#include "render/RenderBufferObject.h"
#include "memory/ResourceWrapper.h"
#include "render/Shader.h"
#include "geometry/ShapeFactory.h"

#include "GL/glew.h"
#include "glm/ext.hpp"
#include "core/Logger.h"

#include "render/RenderCommand.h"
#include "runtime/Entity.h"
#include "component/Component.h"
#include "component/MeshRendererComponent.h"
#include "geometry/Mesh.h"
#include "geometry/Model.h"
#include "runtime/Context.h"
#include "component/RenderableComponent.h"
#include "component/ObjectComponent.h"

#include "core/Engine.h"

ResourceWrapper<Texture> IBL::generateIrradianceMap(ResourceWrapper<Texture> environmentMap, Scene* scene)
{
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Generate Irradiance map");

	auto irradianceShader = Shader::load(SGE_ROOT_DIR "Resources/Engine/Shaders/IrradianceShader.glsl");

	// Generate FBO 
	FrameBufferObject fbo;

	fbo.bind();

	// Generate cubemap
	TextureData textureData;
	textureData.target = TextureTarget::TEXTURE_CUBE_MAP;
	textureData.width = 32;
	textureData.height = 32;
	textureData.channels = 3;
	textureData.internalFormat = TextureInternalFormat::RGB16F;
	textureData.format = TextureFormat::RGB;
	textureData.type = TextureType::FLOAT;
	textureData.filter = TextureFilter::Linear;
	textureData.wrap = TextureWrap::Clamp;
	textureData.genMipMap = false;
	textureData.data = nullptr;
	auto irradianceMap = Texture::createTexture(textureData);

	RenderBufferObject rbo{ 32, 32 };
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
	glViewport(0, 0, 32, 32);

	irradianceShader->use();
	irradianceShader->setProjectionMatrix(captureProjection);
	irradianceShader->setUniformValue("environmentMap", 0);

	environmentMap.get()->setSlot(0);
	environmentMap.get()->bind();


	auto box = ShapeFactory::createBoxEntity(&Engine::get()->getContext()->getRegistry());
	box.RemoveComponent<RenderableComponent>();
	box.RemoveComponent<ObjectComponent>();
	auto vao = box.getComponent<MeshRendererComponent>().mesh.resource()->getPrimaryMesh()->getVAO();

	// render to cube
	// Attach cube map to frame buffer
	for (int i = 0; i < 6; i++)
	{
		// set view
		irradianceShader->setViewMatrix(captureViews[i]);

		// attach cubemap face to fbo
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMap.get()->getID(), 0);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// render cube
		RenderCommand::draw(vao);
	}

	environmentMap.get()->unbind();
	fbo.unbind();

	glPopDebugGroup();

	return irradianceMap;
}

ResourceWrapper<Texture> IBL::generatePrefilterEnvMap(ResourceWrapper<Texture> environmentMap, Scene* scene)
{
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Generate Prefilter Environemnt map");

	auto prefilterShader = Shader::load(SGE_ROOT_DIR "Resources/Engine/Shaders/IBLPrefilterShader.glsl");

	// Generate FBO 
	FrameBufferObject fbo;

	fbo.bind();

	// Generate cubemap
	TextureData textureData2;
	textureData2.target = TextureTarget::TEXTURE_CUBE_MAP;
	textureData2.width = 128;
	textureData2.height = 128;
	textureData2.channels = 3;
	textureData2.internalFormat = TextureInternalFormat::RGB16F;
	textureData2.format = TextureFormat::RGB;
	textureData2.type = TextureType::FLOAT;
	textureData2.filter = TextureFilter::Linear;
	textureData2.wrap = TextureWrap::Clamp;
	textureData2.genMipMap = true;
	textureData2.data = nullptr;
	auto prefilterEnvMap = Texture::createTexture(textureData2);

	RenderBufferObject rbo{ 128, 128 };
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

	prefilterShader->use();
	prefilterShader->setProjectionMatrix(captureProjection);
	prefilterShader->setUniformValue("environmentMap", 0);

	environmentMap.get()->setSlot(0);
	environmentMap.get()->bind();


	auto box = ShapeFactory::createBoxEntity(&Engine::get()->getContext()->getRegistry());
	box.RemoveComponent<RenderableComponent>();
	box.RemoveComponent<ObjectComponent>();
	auto vao = box.getComponent<MeshRendererComponent>().mesh.resource()->getPrimaryMesh()->getVAO();

	// render to cube
	// Attach cube map to frame buffer
	int maxMipLevel = 5;
	for (int mip = 0; mip < maxMipLevel; mip++)
	{
		int mipWidth = 128 * std::pow(.5, mip);
		int mipHeight = 128 * std::pow(.5, mip);

		//rbo.Bind();
		//glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, mipWidth, mipHeight);

		// set viewport
		glViewport(0, 0, mipWidth, mipHeight);
		
		float roughness = (float)mip / (maxMipLevel - 1);
		prefilterShader->setUniformValue("roughness", roughness);

		for (int i = 0; i < 6; i++)
		{
			// set view
			prefilterShader->setViewMatrix(captureViews[i]);

			// attach cubemap face to fbo
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilterEnvMap.get()->getID(), mip);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// render cube
			RenderCommand::draw(vao);
		}
	}

	environmentMap.get()->unbind();
	fbo.unbind();

	glPopDebugGroup();

	return prefilterEnvMap;
}

ResourceWrapper<Texture> IBL::generateBRDFIntegrationLUT(Scene* scene)
{
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Generate BRDF Integration map");

	auto BRDFIntegrationShader = Shader::load(SGE_ROOT_DIR "Resources/Engine/Shaders/BRDFIntegrationShader.glsl");

	// Generate FBO 
	FrameBufferObject fbo;

	fbo.bind();

	// Generate 2D LUT
	TextureData textureData;
	textureData.target = TextureTarget::TEXTURE_2D;
	textureData.width = 512;
	textureData.height = 512;
	textureData.channels = 2;
	textureData.internalFormat = TextureInternalFormat::RG16F;
	textureData.format = TextureFormat::RG;
	textureData.type = TextureType::FLOAT;
	textureData.filter = TextureFilter::Linear;
	textureData.wrap = TextureWrap::Clamp;
	textureData.data = nullptr;
	auto lut = Texture::createTexture(textureData);

	RenderBufferObject rbo{ 512, 512 };
	fbo.attachRenderBuffer(rbo.GetID(), FrameBufferObject::AttachmentType::Depth);

	if (!fbo.isComplete())
	{
		logError("FBO is not complete!");
		return nullptr;
	}

	fbo.attachTexture(lut.get()->getID());

	// set viewport
	glViewport(0, 0, 512, 512);

	BRDFIntegrationShader->use();

	auto quad = ShapeFactory::createQuad(&Engine::get()->getContext()->getRegistry());
	quad.RemoveComponent<RenderableComponent>();
	quad.RemoveComponent<ObjectComponent>();

	auto vao = quad.getComponent<MeshRendererComponent>().mesh.resource()->getPrimaryMesh()->getVAO();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// render to quad
	RenderCommand::draw(vao);

	fbo.unbind();

	glPopDebugGroup();

	return lut;

	
}
