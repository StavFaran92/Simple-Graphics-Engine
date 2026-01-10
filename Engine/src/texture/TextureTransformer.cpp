#include "texture/TextureTransformer.h"

#include "memory/ResourceWrapper.h"
#include "texture/Texture.h"
#include "render/Shader.h"
#include "render/FrameBufferObject.h"
#include "render/RenderBufferObject.h"
#include "geometry/ShapeFactory.h"
#include "core/Engine.h"
#include "runtime/Context.h"
#include "runtime/Entity.h"
#include "component/Component.h"
#include "render/RenderCommand.h"
#include <GL/glew.h>
#include "geometry/MeshCollection.h"
#include "component/MeshRendererComponent.h"
#include "component/ObjectComponent.h"
#include "component/RenderableComponent.h"

ResourceWrapper<Texture> TextureTransformer::flipVertical(ResourceWrapper<Texture> srcTexture)
{
	auto dstTexture = Texture::createTexture(srcTexture.get()->getData());
	flipVertical(srcTexture, dstTexture);
	return dstTexture;
}

void TextureTransformer::flipVertical(ResourceWrapper<Texture> srcTexture, ResourceWrapper<Texture>& dstTexture)
{
	auto shader = Shader::load(SGE_ROOT_DIR "Resources/Engine/Shaders/FlipTextureShader.glsl");

	// Generate FBO 
	FrameBufferObject fbo;

	fbo.bind();

	RenderBufferObject rbo{ srcTexture.get()->getWidth(), srcTexture.get()->getHeight() };
	fbo.attachRenderBuffer(rbo.GetID(), FrameBufferObject::AttachmentType::Depth);

	if (!fbo.isComplete())
	{
		logError("FBO is not complete!");
		return;
	}

	// set viewport
	glViewport(0, 0, srcTexture.get()->getWidth(), srcTexture.get()->getHeight());

	shader->use();
	shader->setUniformValue("source", 0);

	srcTexture.get()->setSlot(0);
	srcTexture.get()->bind();

	fbo.attachTexture(dstTexture.get()->getID());

	auto quad = ShapeFactory::createQuad(&Engine::get()->getContext()->getRegistry());
	quad.RemoveComponent<RenderableComponent>();
	quad.RemoveComponent<ObjectComponent>();
	auto vao = quad.getComponent<MeshRendererComponent>().mesh.get()->getPrimaryMesh()->getVAO();

	RenderCommand::clear();

	// render to quad
	RenderCommand::draw(vao);
}

ResourceWrapper<Texture> TextureTransformer::applyGammaCorrection(ResourceWrapper<Texture> srcTexture)
{
	auto dstTexture = Texture::createTexture(srcTexture.get()->getData());
	applyGammaCorrection(srcTexture, dstTexture);
	return dstTexture;
}

void TextureTransformer::applyGammaCorrection(ResourceWrapper<Texture> srcTexture, ResourceWrapper<Texture>& dstTexture)
{
	auto shader = Shader::load(SGE_ROOT_DIR "Resources/Engine/Shaders/ApplyGammaCorrectionShader.glsl");

	// Generate FBO 
	FrameBufferObject fbo;

	fbo.bind();

	RenderBufferObject rbo{ srcTexture.get()->getWidth(), srcTexture.get()->getHeight() };
	fbo.attachRenderBuffer(rbo.GetID(), FrameBufferObject::AttachmentType::Depth);

	if (!fbo.isComplete())
	{
		logError("FBO is not complete!");
		return;
	}

	// set viewport
	glViewport(0, 0, srcTexture.get()->getWidth(), srcTexture.get()->getHeight());

	shader->use();
	shader->setUniformValue("source", 0);

	srcTexture.get()->setSlot(0);
	srcTexture.get()->bind();

	fbo.attachTexture(dstTexture.get()->getID());

	auto quad = ShapeFactory::createQuad(&Engine::get()->getContext()->getRegistry());
	quad.RemoveComponent<RenderableComponent>();
	quad.RemoveComponent<ObjectComponent>();
	auto vao = quad.getComponent<MeshRendererComponent>().mesh.get()->getPrimaryMesh()->getVAO();

	RenderCommand::clear();

	// render to quad
	RenderCommand::draw(vao);
}
