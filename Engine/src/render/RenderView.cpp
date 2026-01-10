#include "render/RenderView.h"

#include <GL/glew.h>

#include "render/RenderCommand.h"
#include "render/FrameBufferObject.h"
#include "render/RenderBufferObject.h"

RenderTarget::RenderTarget(Viewport viewport)
{
	m_renderTargetFBO = std::make_shared<FrameBufferObject>();
	m_renderTargetRBO = std::make_shared<RenderBufferObject>(viewport.w, viewport.h);

	m_renderTargetFBO->bind();

	Texture::TextureData textureData;
	textureData.target = Texture::TextureTarget::TEXTURE_2D;
	textureData.width = viewport.w;
	textureData.height = viewport.h;
	textureData.channels = 3;
	textureData.internalFormat = Texture::InternalFormat::RGBA8;
	textureData.format = Texture::Format::RGB;
	textureData.type = Texture::Type::UNSIGNED_BYTE;
	textureData.filter = Texture::TextureFilter::Linear;
	textureData.wrap = Texture::TextureWrap::Clamp;
	textureData.data = nullptr;

	m_renderTargetTexture = Texture::createTexture(textureData);
	m_renderTargetFBO->attachTexture(m_renderTargetTexture.get()->getID(), GL_COLOR_ATTACHMENT0);

	unsigned int attachments[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, attachments);

	// Create RBO and attach to FBO
	m_renderTargetFBO->attachRenderBuffer(m_renderTargetRBO->GetID(), FrameBufferObject::AttachmentType::Depth_Stencil);

	if (!m_renderTargetFBO->isComplete())
	{
		logError("FBO is not complete!");
		return;
	}

	m_renderTargetFBO->unbind();
}

RenderView::RenderView(Viewport viewport, const Entity& camera, const std::string& name)
	: m_viewport(viewport), m_camera(camera), m_name(name)
{
	renderTargets[0] = RenderTarget(viewport);
	renderTargets[1] = RenderTarget(viewport);
}

Viewport RenderView::getViewport() const
{
    return m_viewport;
}

void RenderView::setCamera(const Entity& camera)
{
    m_camera = camera;
}

unsigned int RenderView::getRenderTargetTextureID() const
{
	return renderTargets[0].m_renderTargetTexture.get()->getID();
}

ResourceWrapper<Texture> RenderView::getRenderTargetTexture() const
{
	return renderTargets[0].m_renderTargetTexture;
}

unsigned int RenderView::getRenderTargetFrameBufferID() const
{
    return renderTargets[0].m_renderTargetFBO->getID();
}

void RenderView::setTexture(ResourceWrapper<Texture> texture)
{
        renderTargets[0].m_renderTargetFBO->attachTexture(texture.get()->getID(), GL_COLOR_ATTACHMENT0);
}

void RenderView::resize(int w, int h)
{
    m_viewport.w = w;
    m_viewport.h = h;
    renderTargets[0] = RenderTarget(m_viewport);
    renderTargets[1] = RenderTarget(m_viewport);
}

void RenderView::bind()
{
	renderTargets[m_boundTargetTextureSlot].m_renderTargetFBO->bind();
}

void RenderView::unbind()
{
	renderTargets[m_boundTargetTextureSlot].m_renderTargetFBO->unbind();
}

void RenderView::swapToAdditionalTarget()
{
	m_boundTargetTextureSlot = 1;
}

void RenderView::swapBackToMainTarget()
{
	m_boundTargetTextureSlot = 0;
}

void RenderView::swapBackToMainTargetWithCopy()
{
	m_boundTargetTextureSlot = 0;

	renderTargets[0].m_renderTargetFBO->bind();

	//RenderCommand::clear();

	RenderCommand::copyFrameBufferData(renderTargets[1].m_renderTargetFBO->getID(),
		renderTargets[0].m_renderTargetFBO->getID(),
		RenderCommand::BufferBit::DEPTH_BUFFER_BIT | RenderCommand::BufferBit::COLOR_BUFFER_BIT);
}

std::string RenderView::getName() const
{
	return m_name;
}

const Entity& RenderView::getCamera() const
{
    return m_camera;
}


void RenderView::setEnabled(bool isEnabled)
{
	m_isEnabled = isEnabled;
}
bool RenderView::isEnabled() const
{
	return m_isEnabled;
}