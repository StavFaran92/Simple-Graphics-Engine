#pragma once

#include <vector>


#include "render/VertexBufferObject.h"
#include "render/VertexBufferElement.h"
#include "render/ElementBufferObject.h"


struct VertexLayout;

class VertexArrayObject
{
public:
	VertexArrayObject();
	~VertexArrayObject();

	VertexArrayObject(const VertexArrayObject&) = delete;
	VertexArrayObject& operator=(const VertexArrayObject&) = delete;

	void attachBuffer(std::shared_ptr<VertexBufferObject> vbo, const ElementBufferObject* ebo);
	void setVertexCount(unsigned int vCount);
	std::shared_ptr<VertexBufferObject> getBufferByID(unsigned int id) const;

	
	bool build();

	void Bind() const;
	void Unbind() const;
	unsigned int GetIndexCount() const;
	unsigned int GetVerticesCount() const;

	

	

private:
	void fillVertexAttributes(const VertexLayout& layout);
private:
	unsigned int m_id = 0;
	unsigned int m_indexCount = 0;
	unsigned int m_verticesCount = 0;

	std::vector<std::shared_ptr<VertexBufferObject>> m_attachedBuffers;
};
