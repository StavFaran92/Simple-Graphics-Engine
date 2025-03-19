#include "RenderView.h"

#include <GL/glew.h>

RenderView::RenderView(Viewport viewport, const Entity& camera)
	: m_viewport(viewport), m_camera(camera)
{
	m_renderTargetFBO = std::make_shared<FrameBufferObject>();
	m_renderTargetRBO = std::make_shared<RenderBufferObject>(m_viewport.w, m_viewport.h);

	m_renderTargetFBO->bind();

	// Generate Texture for Position data
	m_renderTargetTexture = Texture::createEmptyTexture(m_viewport.w, m_viewport.h);
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

RenderView::Viewport RenderView::getViewport() const
{
    return m_viewport;
}

void RenderView::setCamera(const Entity& camera)
{
    m_camera = camera;
}

unsigned int RenderView::getRenderTargetTextureID() const
{
    return m_renderTargetTexture.get()->getID();
}

unsigned int RenderView::getRenderTargetFrameBufferID() const
{
	return m_renderTargetFBO->getID();
}

void RenderView::setTexture(Resource<Texture> texture)
{
	// TODO Is this OK??
	m_renderTargetFBO->attachTexture(texture.get()->getID(), GL_COLOR_ATTACHMENT0);
}

void RenderView::bind()
{
	m_renderTargetFBO->bind();
}

void RenderView::unbind()
{
	m_renderTargetFBO->unbind();
}

const Entity& RenderView::getCamera() const
{
    return m_camera;
}
