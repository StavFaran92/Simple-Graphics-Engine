#include "VertexBufferObject.h"

#include <GL/glew.h>
#include "Logger.h"

VertexBufferObject::VertexBufferObject(const void* data, unsigned int size, unsigned int sizeOfType)
	: m_length(size)
{
	glGenBuffers(1, &m_id);
	glBindBuffer(GL_ARRAY_BUFFER, m_id);
	glBufferData(GL_ARRAY_BUFFER, size * sizeOfType, data, GL_STATIC_DRAW);
}

VertexBufferObject::~VertexBufferObject()
{
	logInfo( __FUNCTION__ );
	glDeleteBuffers(1, &m_id);
}

void VertexBufferObject::Bind() const
{
	glBindBuffer(GL_ARRAY_BUFFER, m_id);
}

void VertexBufferObject::Unbind() const
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

unsigned int VertexBufferObject::getLength() const
{
	return m_length;
}
