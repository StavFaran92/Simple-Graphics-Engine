#pragma once

#include <iostream>

#include "render/Vertex.h"

class VertexBufferObject
{
public:
	/**
	* data - ptr to data
	* size - total size of the Buffer in bytes
	* length - elemnt count in the buffer (usually num of of vertices)
	*/
	VertexBufferObject(const void* data, unsigned int length, unsigned int size);
	~VertexBufferObject();

	void Bind() const;
	void Unbind() const;

	unsigned int getID() const;

	unsigned int getLength() const;
	unsigned int getSize() const;
private:
	unsigned int m_id = 0;
	unsigned int m_length = 0;
	unsigned int m_size = 0;
};
