#include "utils/EquirectangularToCubemapConverter.h"

#include "texture/Texture.h"
#include "render/FrameBufferObject.h"
#include "render/RenderBufferObject.h"
#include "memory/Resource.h"
#include "render/Shader.h"
#include "geometry/ShapeFactory.h"

#include "GL/glew.h"
#include "glm/ext.hpp"
#include "core/Logger.h"

#include "render/RenderCommand.h"
#include "runtime/Entity.h"
#include "component/Component.h"
#include "geometry/MeshCollection.h"
#include "runtime/Context.h"
#include "texture/Cubemap.h"
#include "texture/TextureTransformer.h"
#include "component/MeshComponent.h"
#include "component/ObjectComponent.h"
#include "component/RenderableComponent.h"

#include "core/Engine.h"


Resource<Texture> EquirectangularToCubemapConverter::fromEquirectangularToCubemap(Resource<Texture> equirectangularTexture)
{
	equirectangularTexture = TextureTransformer::flipVertical(equirectangularTexture);

	auto equirectangularShader = Shader::create(SGE_ROOT_DIR + "Resources/Engine/Shaders/EquirectangularToCubemap.glsl");

	// Generate FBO 
	FrameBufferObject fbo;

	fbo.bind();

	// Generate cubemap
	auto cubemap = Cubemap::createEmptyCubemap(1024, 1024, GL_RGB16F, GL_RGB, GL_FLOAT);

	RenderBufferObject rbo{ 1024, 1024 };
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
	glViewport(0, 0, 1024, 1024);

	equirectangularShader->use();
	equirectangularShader->setProjectionMatrix(captureProjection);
	equirectangularShader->setUniformValue("equirectangularMap", 0);


	
	equirectangularTexture.get()->setSlot(0);
	equirectangularTexture.get()->bind();
	

	auto box = ShapeFactory::createBoxEntity(&Engine::get()->getContext()->getRegistry());
	box.RemoveComponent<RenderableComponent>();
	box.RemoveComponent<ObjectComponent>();
	auto vao = box.getComponent<MeshComponent>().mesh.get()->getPrimaryMesh()->getVAO();

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

Resource<Texture> EquirectangularToCubemapConverter::fromCubemapToEquirectangular(Resource<Texture> cubemapTexture)
{
	auto cubemapToEquirectangularShader = Shader::create(SGE_ROOT_DIR + "Resources/Engine/Shaders/CubemapToEquirectangular.glsl");

	// Generate FBO 
	FrameBufferObject fbo;

	fbo.bind();

	int outputWidth = cubemapTexture.get()->getWidth() * 3;
	int outputHeight = cubemapTexture.get()->getHeight() * 2;

	// Generate cubemap
	auto equirectnagular = Texture::create2DTextureFromBuffer(
		outputWidth,
		outputHeight,
		GL_RGB, GL_RGB, GL_UNSIGNED_BYTE,
		{
			{ GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE },
			{ GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE },
			{ GL_TEXTURE_MIN_FILTER, GL_LINEAR },
			{ GL_TEXTURE_MAG_FILTER, GL_LINEAR }
		}
	, nullptr);

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


	auto quad = ShapeFactory::createQuad(&Engine::get()->getContext()->getRegistry());
	quad.RemoveComponent<RenderableComponent>();
	quad.RemoveComponent<ObjectComponent>();
	auto vao = quad.getComponent<MeshComponent>().mesh.get()->getPrimaryMesh()->getVAO();

	// render to quad
	// attach cubemap face to fbo
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// render cube
	RenderCommand::draw(vao);

	fbo.unbind();

	return equirectnagular;
}
