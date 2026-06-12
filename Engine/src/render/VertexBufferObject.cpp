#include "render/VertexBufferObject.h"

#include <GL/glew.h>
#include "core/Logger.h"

std::shared_ptr<VertexBufferObject> VertexBufferObject::createRaw(const void* data, unsigned int length, unsigned int size, const VertexLayout& layout)
{
	std::shared_ptr<VertexBufferObject> instance = std::make_shared<VertexBufferObject>();
	glGenBuffers(1, &instance->m_id);
	glBindBuffer(GL_ARRAY_BUFFER, instance->m_id);
	glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
	instance->setLayout(layout);
	instance->m_length = length;
	instance->m_size = size;
	return instance;
}

std::shared_ptr<VertexBufferObject> VertexBufferObject::create(const std::vector<StaticVertex>& vertices, const VertexLayout& layout)
{
	VertexLayout instLayout = layout;
	// create raw vertices vector
	// calculate stride
	int stride = 0;
	for (auto entry : layout.attribs)
	{
		auto& attribData = getAttributeData(entry);
		stride += attribData.length * attribData.size;
	}

	// Update layout info
	instLayout.stride = stride;

	std::vector<uint8_t> raw;
	unsigned int bufferSize = stride * vertices.size();
	raw.resize(bufferSize);

	for (size_t i = 0; i < vertices.size(); ++i)
	{
		size_t offset = 0;
		uint8_t* dest = raw.data() + i * stride;

		for (auto entry : layout.attribs)
		{
			auto& attribData = getAttributeData(entry);
			const StaticVertex& v = vertices[i];

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
			else if (entry == LayoutAttribute::Tangents)
			{
				memcpy(dest + offset + attribData.size * 0, &v.tangent.x, attribData.size);
				memcpy(dest + offset + attribData.size * 1, &v.tangent.y, attribData.size);
			}
			//else if (entry == LayoutAttribute::BoneIDs)
			//{
			//	memcpy(dest + offset + attribData.size * 0, &v.boneIDs.x, attribData.size);
			//	memcpy(dest + offset + attribData.size * 1, &v.boneIDs.y, attribData.size);
			//	memcpy(dest + offset + attribData.size * 2, &v.boneIDs.z, attribData.size);
			//}
			//else if (entry == LayoutAttribute::BoneWeights)
			//{
			//	memcpy(dest + offset + attribData.size * 0, &v.boneWeights.x, attribData.size);
			//	memcpy(dest + offset + attribData.size * 1, &v.boneWeights.y, attribData.size);
			//	memcpy(dest + offset + attribData.size * 2, &v.boneWeights.z, attribData.size);
			//}

			offset += attribData.length * attribData.size;
		}
	}

	// create VBO & Fill content
	return VertexBufferObject::createRaw(&(raw[0]), vertices.size(), bufferSize, instLayout);
}

VertexBufferObject::~VertexBufferObject()
{
	logDebug( __FUNCTION__ );
	glDeleteBuffers(1, &m_id);
}

void VertexBufferObject::Bind() const
{
	glBindBuffer(GL_ARRAY_BUFFER, m_id);
}

void VertexBufferObject::Unbind() const
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

unsigned int VertexBufferObject::getID() const
{
	return m_id;
}

unsigned int VertexBufferObject::getLength() const
{
	return m_length;
}

unsigned int VertexBufferObject::getSize() const
{
	return m_size;
}

const VertexLayout& VertexBufferObject::getLayout() const
{
	return m_layout;
}

void VertexBufferObject::setLayout(const VertexLayout& layout)
{
	m_layout = layout;
}

void VertexBufferObject::addVertex(const StaticVertex& v)
{
	m_vertices.push_back(v);
}

void VertexBufferObject::addVertices(const std::vector<StaticVertex>& vs)
{
	m_vertices.reserve(vs.size());
	for (const auto& v : vs)
	{
		addVertex(v);
	}
}