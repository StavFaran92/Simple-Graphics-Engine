#include "render/IBL.h"

#include "Texture.h"
#include "render/FrameBufferObject.h"
#include "render/RenderBufferObject.h"
#include "Resource.h"
#include "Shader.h"
#include "ShapeFactory.h"

#include "GL/glew.h"
#include "glm/ext.hpp"
#include "systems/Logger.h"

#include "render/RenderCommand.h"
#include "Entity.h"
#include "Component.h"
#include "Mesh.h"
#include "geometry/MeshCollection.h"
#include "Context.h"
#include "texture/Cubemap.h"

#include "core/Engine.h"

Resource<Texture> IBL::generateIrradianceMap(Resource<Texture> environmentMap, Scene* scene)
{
	auto irradianceShader = Shader::create(SGE_ROOT_DIR + "Resources/Engine/Shaders/IrradianceShader.glsl");

	// Generate FBO 
	FrameBufferObject fbo;

	fbo.bind();

	// Generate cubemap
	auto irradianceMap = Cubemap::createEmptyCubemap(32, 32, GL_RGB16F, GL_RGB, GL_FLOAT);

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
	auto vao = box.getComponent<MeshComponent>().mesh.get()->getPrimaryMesh()->getVAO();

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

	return irradianceMap;
}

Resource<Texture> IBL::generatePrefilterEnvMap(Resource<Texture> environmentMap, Scene* scene)
{
	auto prefilterShader = Shader::create(SGE_ROOT_DIR + "Resources/Engine/Shaders/IBLPrefilterShader.glsl");

	// Generate FBO 
	FrameBufferObject fbo;

	fbo.bind();

	// Generate cubemap
	auto prefilterEnvMap = Cubemap::createEmptyCubemap(128, 128, GL_RGB16F, GL_RGB, GL_FLOAT, {
		{ GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR },
		{ GL_TEXTURE_MAG_FILTER, GL_LINEAR },
		{ GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE },
		{ GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE },
		{ GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE }
	}, true);

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
	auto vao = box.getComponent<MeshComponent>().mesh.get()->getPrimaryMesh()->getVAO();

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

	return prefilterEnvMap;
}

Resource<Texture> IBL::generateBRDFIntegrationLUT(Scene* scene)
{
	auto BRDFIntegrationShader = Shader::create(SGE_ROOT_DIR + "Resources/Engine/Shaders/BRDFIntegrationShader.glsl");

	// Generate FBO 
	FrameBufferObject fbo;

	fbo.bind();

	// Generate 2D LUT
	auto lut = Texture::create2DTextureFromBuffer(512, 512, GL_RG16F, GL_RG, GL_FLOAT, 
		{	
			{ GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE },
			{ GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE },
			{ GL_TEXTURE_MIN_FILTER, GL_LINEAR },
			{ GL_TEXTURE_MAG_FILTER, GL_LINEAR } 
		}
	, nullptr);

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

	auto vao = quad.getComponent<MeshComponent>().mesh.get()->getPrimaryMesh()->getVAO();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// render to quad
	RenderCommand::draw(vao);

	fbo.unbind();

	return lut;

	
}
