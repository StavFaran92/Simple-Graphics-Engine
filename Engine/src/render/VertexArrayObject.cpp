#include "render/VertexArrayObject.h"

#include <gl/glew.h>

#include "Logger.h"
#include "VertexLayout.h"

VertexArrayObject::VertexArrayObject()
{
	glGenVertexArrays(1, &m_id);
	glBindVertexArray(m_id);
}

VertexArrayObject::~VertexArrayObject()
{
	logInfo( __FUNCTION__ );
	glDeleteVertexArrays(1, &m_id);
}

void VertexArrayObject::attachBuffer(std::shared_ptr<VertexBufferObject> vbo, const ElementBufferObject* ebo)
{
	// bind this VAO
	Bind();

	m_attachedBuffers.push_back(vbo);

	// bind VBO to associate with this VAO
	vbo->Bind();

	// bind IBO to associate with this VAO
	if (ebo)
	{
		ebo->Bind();
		m_indexCount = ebo->getLength();
	}
}

void VertexArrayObject::setVertexCount(unsigned int vCount)
{
	m_verticesCount = vCount;
}

void VertexArrayObject::fillVertexAttributes(const VertexLayout& layout)
{
	int offset = 0;
	for (auto entry : layout.attribs)
	{
		AttributeData& attribData = getAttributeData(entry);
		glEnableVertexAttribArray(attribData.location);
		if (attribData.typeName == typeid(float).name())
		{
			glVertexAttribPointer(attribData.location, attribData.length, GL_FLOAT, GL_FALSE, layout.stride, (void*)offset);
		}
		else if (attribData.typeName == typeid(int).name())
		{
			glVertexAttribIPointer(attribData.location, attribData.length, GL_INT, layout.stride, (void*)offset);
		}

		offset += attribData.size * attribData.length;
	}
}

bool VertexArrayObject::build()
{
	for (auto& buffer : m_attachedBuffers)
	{
		buffer->Bind();
		auto& layout = buffer->getLayout();
		fillVertexAttributes(layout);
		buffer->Unbind();
	}

	return true;
}
void VertexArrayObject::Bind() const
{
	glBindVertexArray(m_id);
}

void VertexArrayObject::Unbind() const
{
	glBindVertexArray(0);
}

unsigned int VertexArrayObject::GetIndexCount() const
{
	return m_indexCount;
}

unsigned int VertexArrayObject::GetVerticesCount() const
{
	return m_verticesCount;
}

std::shared_ptr<VertexBufferObject> VertexArrayObject::getBufferByID(unsigned int id) const
{
	for (auto& buffer : m_attachedBuffers)
	{
		if (buffer->getID() == id)
		{
			return buffer;
		}
	}
}
