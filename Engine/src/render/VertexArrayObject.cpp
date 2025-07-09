#include "render/VertexArrayObject.h"

#include <gl/glew.h>

#include "systems/Logger.h"
#include "render/VertexLayout.h"

VertexArrayObject::VertexArrayObject()
	: m_layout()
{
	glGenVertexArrays(1, &m_id);
	glBindVertexArray(m_id);
}

VertexArrayObject::VertexArrayObject(const VertexLayout& layout)
	: m_layout(layout)
{
	glGenVertexArrays(1, &m_id);
	glBindVertexArray(m_id);
}

VertexArrayObject::~VertexArrayObject()
{
	logInfo( __FUNCTION__ );
	glDeleteVertexArrays(1, &m_id);
}

void VertexArrayObject::AttachBuffer(const VertexBufferObject& vbo, const ElementBufferObject* ebo)
{
	// bind this VAO
	Bind();

	// bind VBO to associate with this VAO
	vbo.Bind();
	m_verticesCount = vbo.getLength();

	// bind IBO to associate with this VAO
	if (ebo)
	{
		ebo->Bind();
		m_indexCount = ebo->getLength();
	}

	fillVertexAttributes();
}

void VertexArrayObject::fillVertexAttributes()
{
	int offset = 0;
	for (auto entry : m_layout.attribs)
	{
		auto& attribData = getAttributeData(entry);
		glEnableVertexAttribArray(attribData.location);
		if (attribData.typeName == typeid(float).name())
		{
			glVertexAttribPointer(attribData.location, attribData.length, GL_FLOAT, GL_FALSE, m_layout.stride, (void*)offset);
		}
		else if (attribData.typeName == typeid(int).name())
		{
			glVertexAttribIPointer(attribData.location, attribData.length, GL_INT, m_layout.stride, (void*)offset);
		}

		offset += attribData.size * attribData.length;
	}
}

void VertexArrayObject::setLayout(const VertexLayout& layout)
{
	m_layout = layout;
}

void VertexArrayObject::addVertex(const Vertex& v)
{
	m_vertices.push_back(v);
}

void VertexArrayObject::addVertices(const std::vector<Vertex>& vs)
{
	m_vertices.reserve(vs.size());
	for (const auto& v : vs)
	{
		addVertex(v);
	}
}

bool VertexArrayObject::build()
{
	// create raw vertices vector
	// calculate stride
	int stride = 0;
	for (auto entry : m_layout.attribs)
	{
		auto& attribData = getAttributeData(entry);
		stride += attribData.length * attribData.size;
	}

	// Update layout info
	m_layout.stride = stride;

	// Create verticies array
	// array size = size of each attribute * size of elements in attribute * vertices count
	//int offset = 0;
	//unsigned char* vertices = new unsigned char[bufferSize];

	std::vector<uint8_t> raw;
	unsigned int bufferSize = stride * m_vertices.size();
	raw.resize(bufferSize);

	for (size_t i = 0; i < m_vertices.size(); ++i)
	{
		size_t offset = 0;
		uint8_t* dest = raw.data() + i * m_layout.stride;

		for (auto entry : m_layout.attribs)
		{
			auto& attribData = getAttributeData(entry);
			const Vertex& v = m_vertices[i];

			if (entry == LayoutAttribute::Positions)
			{
				memcpy(dest + offset + attribData.size * 0, &v.position.x, attribData.size);
				memcpy(dest + offset + attribData.size * 1, &v.position.y, attribData.size);
				memcpy(dest + offset + attribData.size * 2, &v.position.z, attribData.size);
			}
			else if (entry == LayoutAttribute::Normals)
			{
				memcpy(dest + offset + attribData.size * 0, &v.normal.x, attribData.size);
				memcpy(dest + offset + attribData.size * 1, &v.normal.y, attribData.size);
				memcpy(dest + offset + attribData.size * 2, &v.normal.z, attribData.size);
			}
			else if (entry == LayoutAttribute::Texcoords)
			{
				memcpy(dest + offset + attribData.size * 0, &v.texCoord.x, attribData.size);
				memcpy(dest + offset + attribData.size * 1, &v.texCoord.y, attribData.size);
			}
			else if (entry == LayoutAttribute::Colors)
			{
				memcpy(dest + offset + attribData.size * 0, &v.color.x, attribData.size);
				memcpy(dest + offset + attribData.size * 1, &v.color.y, attribData.size);
				memcpy(dest + offset + attribData.size * 2, &v.color.z, attribData.size);
			}
			else if (entry == LayoutAttribute::Tangents)
			{
				memcpy(dest + offset + attribData.size * 0, &v.tangent.x, attribData.size);
				memcpy(dest + offset + attribData.size * 1, &v.tangent.y, attribData.size);
			}
			else if (entry == LayoutAttribute::BoneIDs)
			{
				memcpy(dest + offset + attribData.size * 0, &v.boneIDs.x, attribData.size);
				memcpy(dest + offset + attribData.size * 1, &v.boneIDs.y, attribData.size);
				memcpy(dest + offset + attribData.size * 2, &v.boneIDs.z, attribData.size);
			}
			else if (entry == LayoutAttribute::BoneWeights)
			{
				memcpy(dest + offset + attribData.size * 0, &v.boneWeights.x, attribData.size);
				memcpy(dest + offset + attribData.size * 1, &v.boneWeights.y, attribData.size);
				memcpy(dest + offset + attribData.size * 2, &v.boneWeights.z, attribData.size);
			}

			offset += attribData.length * attribData.size;
		}
	}

	// create VBO & Fill content
	m_vbo = std::make_shared<VertexBufferObject>(&(raw[0]), m_vertices.size(), bufferSize);

	// fill vertex attributes
	AttachBuffer(*m_vbo.get(), 0);


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