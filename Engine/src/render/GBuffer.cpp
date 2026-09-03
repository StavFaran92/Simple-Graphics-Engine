#include "render/GBuffer.h"

#include "gl/glew.h"

#include "core/Logger.h"
#include "texture/Texture.h"

namespace
{
	uint32_t toGLAttachment(GBuffer::Attachment attachment)
	{
		return GL_COLOR_ATTACHMENT0 + static_cast<uint32_t>(attachment);
	}
}

bool GBuffer::setup(int width, int height)
{
	m_fbo.bind();

	setTexture(Attachment::Position, Texture::createTexture(width, height, 3, TextureInternalFormat::RGBA16F, TextureFormat::RGBA, TextureType::FLOAT));
	setTexture(Attachment::Normal, Texture::createTexture(width, height, 3, TextureInternalFormat::RGBA16F, TextureFormat::RGBA, TextureType::FLOAT));
	setTexture(Attachment::Albedo, Texture::createTexture(width, height, 3, TextureInternalFormat::RGBA, TextureFormat::RGBA, TextureType::UNSIGNED_BYTE));
	setTexture(Attachment::MRA, Texture::createTexture(width, height, 3, TextureInternalFormat::RGBA, TextureFormat::RGBA, TextureType::UNSIGNED_BYTE));
	setTexture(Attachment::PositionVS, Texture::createTexture(width, height, 3, TextureInternalFormat::RGBA16F, TextureFormat::RGBA, TextureType::FLOAT));
	setTexture(Attachment::NormalVS, Texture::createTexture(width, height, 3, TextureInternalFormat::RGBA16F, TextureFormat::RGBA, TextureType::FLOAT));
	setTexture(Attachment::Tangent, Texture::createTexture(width, height, 3, TextureInternalFormat::RGBA16F, TextureFormat::RGBA, TextureType::FLOAT));

	m_fbo.bind();

	constexpr size_t attachmentCount = (size_t)Attachment::Count;
	uint32_t attachments[attachmentCount];
	for (size_t i = 0; i < attachmentCount; i++)
	{
		attachments[i] = GL_COLOR_ATTACHMENT0 + (uint32_t)i;
	}
	glDrawBuffers((GLsizei)attachmentCount, attachments);

	m_renderBuffer = RenderBufferObject(width, height);
	m_fbo.attachRenderBuffer(m_renderBuffer.GetID(), FrameBufferObject::AttachmentType::Depth_Stencil);

	if (!m_fbo.isComplete())
	{
		logError("GBuffer FBO is not complete!");
		return false;
	}

	m_fbo.unbind();

	m_isReady = true;

	return true;
}

void GBuffer::bind()
{
	m_fbo.bind();
}

void GBuffer::unbind()
{
	m_fbo.unbind();
}

uint32_t GBuffer::getID() const
{
	return m_fbo.getID();
}

TextureResourceRef GBuffer::getTexture(Attachment attachment) const
{
	return m_textures[(size_t)attachment];
}

void GBuffer::setTexture(Attachment attachment, TextureResourceRef texture)
{
	m_textures[(size_t)attachment] = texture;

	m_fbo.bind();
	m_fbo.attachTexture(texture.get()->getID(), toGLAttachment(attachment));
}
