#include "FrameBufferObject.h"

#include "GL/glew.h"
#include "Logger.h"

#define VALIDATE_BIND if (s_boundFBO != m_id) { logWarning("Frame buffer object is currently not bound"); }

uint32_t FrameBufferObject::s_boundFBO = 0;

FrameBufferObject::FrameBufferObject()
{
	glGenFramebuffers(1, &m_id);
}

FrameBufferObject::~FrameBufferObject()
{
	glDeleteFramebuffers(1, &m_id);
}

void FrameBufferObject::bind()
{
	s_boundFBO = m_id;

	glBindFramebuffer(GL_FRAMEBUFFER, m_id);
}

void FrameBufferObject::unbind()
{
	s_boundFBO = 0;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

bool FrameBufferObject::isComplete()
{
	VALIDATE_BIND;

	bool result = false;

	result = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;

	return result;

}

uint32_t FrameBufferObject::getID() const
{
	return m_id;
}

void FrameBufferObject::attachRenderBuffer(uint32_t renderBufferID, AttachmentType type)
{
	VALIDATE_BIND;

	GLint attachment = GL_FRAMEBUFFER_DEFAULT;
	switch (type)
	{
	case AttachmentType::Color:
		attachment = GL_COLOR_ATTACHMENT0;
		break;
	case AttachmentType::Depth:
		attachment = GL_DEPTH_ATTACHMENT;
		break;
	case AttachmentType::Stencil:
		attachment = GL_STENCIL_ATTACHMENT;
		break;
	case AttachmentType::Depth_Stencil:
		attachment = GL_DEPTH_STENCIL_ATTACHMENT;
		break;
	}

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, renderBufferID);
}

void FrameBufferObject::attachTexture(uint32_t textureID)
{
	VALIDATE_BIND;

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureID, 0);
}

void FrameBufferObject::attachTexture(uint32_t textureID, uint32_t attachementType)
{
	VALIDATE_BIND;

	glFramebufferTexture2D(GL_FRAMEBUFFER, attachementType, GL_TEXTURE_2D, textureID, 0);
}
