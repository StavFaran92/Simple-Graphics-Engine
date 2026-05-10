#include "texture/TextureTransformer.h"

#include "memory/ResourceRef.h"
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
#include "geometry/Model.h"
#include "component/MeshRendererComponent.h"
#include "component/ObjectComponent.h"
#include "component/RenderableComponent.h"
#include "core/Logger.h"
#include "memory/BuiltInAssets.h"

TextureResourceRef TextureTransformer::flipVertical(TextureResourceRef srcTexture)
{
	auto dstTexture = srcTexture->clone();
	//auto dstTexture = Texture::createTexture(srcTexture.get()->getData());
	flipVertical(srcTexture, dstTexture);
	return dstTexture;
}

void TextureTransformer::flipVertical(TextureResourceRef srcTexture, TextureResourceRef& dstTexture)
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
	auto vao = quad.getComponent<MeshRendererComponent>().mesh.resource()->getPrimaryMesh()->getVAO();

	RenderCommand::clear();

	// render to quad
	RenderCommand::draw(vao);
}

TextureResourceRef TextureTransformer::applyGammaCorrection(TextureResourceRef srcTexture)
{
	auto dstTexture = srcTexture->clone();
	applyGammaCorrection(srcTexture, dstTexture);
	return dstTexture;
}

void TextureTransformer::applyGammaCorrection(TextureResourceRef srcTexture, TextureResourceRef& dstTexture)
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
	auto vao = quad.getComponent<MeshRendererComponent>().mesh.resource()->getPrimaryMesh()->getVAO();

	RenderCommand::clear();

	// render to quad
	RenderCommand::draw(vao);
}

TextureResourceRef TextureTransformer::packTextures(
	TextureResourceRef src0, int channel0, 
	TextureResourceRef src1, int channel1, 
	TextureResourceRef src2, int channel2, 
	TextureResourceRef src3, int channel3)
{
	auto shader = Shader::load(SGE_ROOT_DIR "Resources/Engine/Shaders/PackTexturesShader.glsl");

	// Generate FBO 
	FrameBufferObject fbo;

	fbo.bind();

	int maxWidth  = std::max({ src0.get()->getWidth(),  src1.get()->getWidth(),  src2.get()->getWidth(),  src3.get()->getWidth() });
	int maxHeight = std::max({ src0.get()->getHeight(), src1.get()->getHeight(), src2.get()->getHeight(), src3.get()->getHeight() });

	RenderBufferObject rbo{ maxWidth, maxHeight };
	fbo.attachRenderBuffer(rbo.GetID(), FrameBufferObject::AttachmentType::Depth);

	TextureData tData;
	tData.channels = 4;
	tData.height = maxHeight;
	tData.width = maxWidth;
	tData.internalFormat = TextureInternalFormat::RGBA8;
	tData.format = TextureFormat::RGBA;
	TextureResourceRef outputTexture = Texture::createTexture(tData);

	fbo.attachTexture(outputTexture->getID());

	if (!fbo.isComplete())
	{
		logError("FBO is not complete!");
		return TextureResourceRef::empty;
	}

	// set viewport
	glViewport(0, 0, maxWidth, maxHeight);

	shader->use();
	shader->setUniformValue("channel0", channel0);
	shader->setUniformValue("channel1", channel1);
	shader->setUniformValue("channel2", channel2);
	shader->setUniformValue("channel3", channel3);

	src0->setSlot(0);
	src0->bind();
	shader->setUniformValue("sampler0", 0);

	src1->setSlot(1);
	src1->bind();
	shader->setUniformValue("sampler1", 1);

	src2->setSlot(2);
	src2->bind();
	shader->setUniformValue("sampler2", 2);

	src3->setSlot(3);
	src3->bind();
	shader->setUniformValue("sampler3", 3);

	RenderCommand::clear();

	// render to quad
	auto quad = BuiltInAssets::getByName<ModelAsset>(SGE_MESH_QUAD).resource();
	RenderCommand::draw(quad->getPrimaryMesh()->getVAO());

	return outputTexture;
}
