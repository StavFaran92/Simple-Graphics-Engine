#pragma once
#include <vector>
#include "VertexBufferElement.h"
#include <GL/glew.h>

class VertexBufferLayout
{
public:
	VertexBufferLayout() :
		m_stride(0) {}
	~VertexBufferLayout();

	template<typename T>
	void Push(unsigned int count)
	{
		//static_assert(false);
	}

	template<>
	void Push<float>(unsigned int count)
	{
		m_elements.push_back({ GL_FLOAT, count, GL_FALSE });
		m_stride += sizeof(GLfloat) * count;
	}

	template<>
	void Push<unsigned int>(unsigned int count)
	{
		m_elements.push_back({ GL_UNSIGNED_INT, count, GL_FALSE });
		m_stride += sizeof(GLuint) * count;
	}

	template<>
	void Push<unsigned char>(unsigned int count)
	{
		m_elements.push_back({ GL_UNSIGNED_BYTE, count, GL_FALSE });
		m_stride += sizeof(GLbyte) * count;
	}

	inline unsigned int GetStride() const { return m_stride; }
	inline const std::vector<VertexBufferElement>& GetElements() const { return m_elements; }

private:
	std::vector<VertexBufferElement> m_elements;
	unsigned int m_stride = 0;
};
