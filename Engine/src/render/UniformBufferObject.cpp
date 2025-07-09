#include "UniformBufferObject.h"

#include <GL/glew.h>

UniformBufferObject::UniformBufferObject(int memsize)
{
	// Generate buffer 
	glGenBuffers(1, &m_id);

	// Allocate data
	bind();
	glBufferData(GL_UNIFORM_BUFFER, memsize, 0, GL_STATIC_DRAW);
	unbind();
}

UniformBufferObject::~UniformBufferObject()
{
	glDeleteBuffers(1, &m_id);
}

void UniformBufferObject::attachToBindPoint(int bindingPointIndex)
{
	glBindBufferBase(GL_UNIFORM_BUFFER, bindingPointIndex, m_id);
}

void UniformBufferObject::setData(int offset, int size, const void* data)
{
	glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
}

void UniformBufferObject::bind() const
{
	glBindBuffer(GL_UNIFORM_BUFFER, m_id);
}

void UniformBufferObject::unbind() const
{
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}
