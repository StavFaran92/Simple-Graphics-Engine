#pragma once

#include <vector>
#include "render/Vertex.h"

#include "render/VertexBufferObject.h"

//#include "render/VertexBufferLayout.h"
#include "render/VertexBufferElement.h"
#include "render/ElementBufferObject.h"
#include "VertexLayout.h"

struct VertexLayout;

class VertexArrayObject
{
public:
	VertexArrayObject();
	VertexArrayObject(const VertexLayout& layout);
	~VertexArrayObject();

	void AttachBuffer(const VertexBufferObject& vbo, const ElementBufferObject* ebo);
	

	void setLayout(const VertexLayout& layout);
	void addVertex(const Vertex& v);
	void addVertices(const std::vector<Vertex>& v);
	bool build();

	void Bind() const;
	void Unbind() const;
	unsigned int GetIndexCount() const;
	unsigned int GetVerticesCount() const;

private:
	void fillVertexAttributes();
private:
	unsigned int m_id = 0;
	unsigned int m_indexCount = 0;
	unsigned int m_verticesCount = 0;

	VertexLayout m_layout;
	std::vector<Vertex> m_vertices;
	std::shared_ptr<VertexBufferObject> m_vbo;
};
