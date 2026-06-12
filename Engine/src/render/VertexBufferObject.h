#pragma once

#include <iostream>

#include "render/Vertex.h"
#include "render/VertexLayout.h"

class VertexBufferObject
{
public:
	static std::shared_ptr<VertexBufferObject> createRaw(const void* data, unsigned int length, unsigned int size, const VertexLayout& layout);
	static std::shared_ptr<VertexBufferObject> create(const std::vector<StaticVertex>& v, const VertexLayout& layout);
	/**
	* data - ptr to data
	* size - total size of the Buffer in bytes
	* length - elemnt count in the buffer (usually num of of vertices)
	*/
	
	~VertexBufferObject();

	void Bind() const;
	void Unbind() const;

	unsigned int getID() const;
	unsigned int getLength() const;
	unsigned int getSize() const;
	const VertexLayout& getLayout() const;
	VertexBufferObject() = default;
private:
	void addVertex(const StaticVertex& v);
	void addVertices(const std::vector<StaticVertex>& v);
	void setLayout(const VertexLayout& layout);
private:
	unsigned int m_id = 0;
	unsigned int m_length = 0;
	unsigned int m_size = 0;

	std::vector<StaticVertex> m_vertices;
	VertexLayout m_layout;
};
