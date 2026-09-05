#include "SSBO.h"

#include <GL/glew.h>

SSBO::SSBO()
{
	glGenBuffers(1, &id);
}

SSBO::~SSBO()
{
	glDeleteBuffers(1, &id);
}

void SSBO::allocate(int size)
{
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
	glBufferData(GL_SHADER_STORAGE_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
}

void SSBO::setData(int size, const void* data)
{
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, size, data);
}

void SSBO::bind()
{
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, slot, id);
}

void SSBO::setSlot(int index)
{
	slot = index;
}
