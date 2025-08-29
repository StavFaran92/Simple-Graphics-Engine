#include "texture/TextureTransformer.h"

#include "memory/Resource.h"
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
#include "component/MeshComponent.h"
#include "component/ObjectComponent.h"
#include "component/RenderableComponent.h"

Resource<Texture> TextureTransformer::flipVertical(Resource<Texture> srcTexture)
{
	auto dstTexture = Texture::createEmptyTexture(srcTexture.get()->getWidth(), srcTexture.get()->getHeight(), GL_RGB, GL_RGB, GL_UNSIGNED_BYTE);
	flipVertical(srcTexture, dstTexture);
	return dstTexture;
}

void TextureTransformer::flipVertical(Resource<Texture> srcTexture, Resource<Texture>& dstTexture)
{
	auto shader = Shader::loadTransient(SGE_ROOT_DIR + "Resources/Engine/Shaders/TextureTransformShader.glsl");

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
	auto vao = quad.getComponent<MeshComponent>().mesh.get()->getPrimaryMesh()->getVAO();

	RenderCommand::clear();

	// render to quad
	RenderCommand::draw(vao);
}