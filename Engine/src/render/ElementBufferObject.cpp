#include "ElementBufferObject.h"

#include <GL/glew.h>

#include "Logger.h"

ElementBufferObject::ElementBufferObject(unsigned int* data, unsigned int length)
	:m_length(length)
{
	glGenBuffers(1, &m_id);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, length * sizeof(unsigned int), data, GL_STATIC_DRAW);
}

ElementBufferObject::~ElementBufferObject()
{
	logInfo( __FUNCTION__ );
	glDeleteBuffers(1, &m_id);
}

void ElementBufferObject::Bind() const
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id);
}

void ElementBufferObject::Unbind() const
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

unsigned int ElementBufferObject::getLength() const
{
	return m_length;
}