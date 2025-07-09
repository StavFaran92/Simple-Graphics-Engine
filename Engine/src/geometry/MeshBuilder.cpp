#include "MeshBuilder.h"

#include "Logger.h"
#include "VertexLayout.h"
#include "Factory.h"
#include "MeshSerializer.h"
#include "Context.h"
#include "ProjectAssetRegistry.h"

MeshBuilder& MeshBuilder::addPosition(const glm::vec3& position)
{
	m_data.m_positions.push_back(position);

	return *this;
}

MeshBuilder& MeshBuilder::addPositions(const std::vector<glm::vec3>& positions)
{
	m_data.m_positions.insert(m_data.m_positions.end(), positions.begin(), positions.end());

	return *this;
}

MeshBuilder& MeshBuilder::addPositions(const float* positions, size_t size)
{
	for (int i = 0; i < size; i++)
	{
		addPosition({ positions[i * 3 + 0], positions[i * 3 + 1], positions[i * 3 + 2] });
	}

	return *this;
}

MeshBuilder& MeshBuilder::addNormal(const glm::vec3& normal)
{
	m_data.m_normals.push_back(normal);

	return *this;
}

MeshBuilder& MeshBuilder::addNormals(const std::vector<glm::vec3>& normals)
{
	m_data.m_normals.insert(m_data.m_normals.end(), normals.begin(), normals.end());

	return *this;
}

MeshBuilder& MeshBuilder::addNormals(const float* normals, size_t size)
{
	for (int i = 0; i < size; i++)
	{
		addNormal({ normals[i * 3 + 0], normals[i * 3 + 1], normals[i * 3 + 2] });
	}

	return *this;
}

MeshBuilder& MeshBuilder::addTexcoord(const glm::vec2& texCoord)
{
	m_data.m_texCoords.push_back(texCoord);

	return *this;
}

MeshBuilder& MeshBuilder::addTexcoords(const std::vector<glm::vec2>& texCoords)
{
	m_data.m_texCoords.insert(m_data.m_texCoords.end(), texCoords.begin(), texCoords.end());

	return *this;
}

MeshBuilder& MeshBuilder::addTexcoords(const float* texCoords, size_t size)
{
	for (int i = 0; i < size; i++)
	{
		addTexcoord({ texCoords[i * 2 + 0], texCoords[i * 2 + 1] });
	}

	return *this;
}

MeshBuilder& MeshBuilder::addColor(const glm::vec3& color)
{
	m_data.m_colors.push_back(color);

	return *this;
}

MeshBuilder& MeshBuilder::addColors(const std::vector<glm::vec3>& colors)
{
	m_data.m_colors.insert(m_data.m_colors.end(), colors.begin(), colors.end());

	return *this;
}

MeshBuilder& MeshBuilder::addColors(const float* colors, size_t size)
{
	for (int i = 0; i < size; i++)
	{
		addColor({ colors[i * 3 + 0], colors[i * 3 + 1], colors[i * 3 + 2] });
	}

	return *this;
}

MeshBuilder& MeshBuilder::addTangent(const glm::vec3& tangent)
{
	m_data.m_tangents.push_back(tangent);

	return *this;
}

MeshBuilder& MeshBuilder::addTangents(const std::vector<glm::vec3>& tangents)
{
	m_data.m_tangents.insert(m_data.m_tangents.end(), tangents.begin(), tangents.end());

	return *this;
}

MeshBuilder& MeshBuilder::addTangents(const float* tangents, size_t size)
{
	for (int i = 0; i < size; i++)
	{
		addTangent({ tangents[i * 3 + 0], tangents[i * 3 + 1], tangents[i * 3 + 2] });
	}

	return *this;
}

MeshBuilder& MeshBuilder::addIndex(unsigned int index)
{
	m_data.m_indices.push_back(index);

	return *this;
}

MeshBuilder& MeshBuilder::addIndices(const unsigned int* indices, size_t size)
{
	int offset = m_data.m_indices.size() > 0 ? m_data.m_indices[m_data.m_indices.size() - 1] + 1 : 0;
	for (int i = 0; i < size; i++)
	{
		addIndex(indices[i] + offset);
	}

	return *this;
}

MeshBuilder& MeshBuilder::addBoneIDs(const glm::ivec3& boneIDs)
{
	m_data.bonesIDs.push_back(boneIDs);

	return *this;
}

MeshBuilder& MeshBuilder::addBoneIDs(const std::vector<glm::ivec3>& bonesIDs)
{
	m_data.bonesIDs.insert(m_data.bonesIDs.end(), bonesIDs.begin(), bonesIDs.end());

	return *this;
}

MeshBuilder& MeshBuilder::addBoneWeights(const glm::vec3& boneWeight)
{
	m_data.bonesWeights.push_back(boneWeight);

	return *this;
}

MeshBuilder& MeshBuilder::addBoneWeights(const std::vector<glm::vec3>& boneWeights)
{
	m_data.bonesWeights.insert(m_data.bonesWeights.end(), boneWeights.begin(), boneWeights.end());

	return *this;
}

MeshBuilder& MeshBuilder::addIndices(const std::vector<unsigned int>& indices)
{
	addIndices(&indices.data()[0], indices.size());

	return *this;
}

MeshBuilder& MeshBuilder::merge(const MeshBuilder& other)
{

	addPositions(other.m_data.m_positions);
	addNormals(other.m_data.m_normals);
	addTexcoords(other.m_data.m_texCoords);
	addColors(other.m_data.m_colors);
	addIndices(other.m_data.m_indices);

	m_data.m_layout = other.m_data.m_layout;

	return *this;
}

MeshBuilder& MeshBuilder::addRawVertices(const float* vertices, VertexLayout layout)
{
	if (!vertices)
	{
		logError("Specified ptr is null");
		return *this;
	}

	if (layout.numOfVertices == 0)
	{
		logError("Size cannot be set to 0.");
		return *this;
	}

	// calculate stride
	int stride = 0;
	for (auto entry : layout.attribs)
	{
		stride += getAttributeCompCount(entry);
	}

	int offset = 0;

	// Parse vertices
	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> texcoords;
	std::vector<glm::vec3> colors;
	std::vector<glm::vec3> tangents;

	for (auto entry : layout.attribs)
	{
		// Parse positions
		if (LayoutAttribute::Positions == entry)
		{
			positions.reserve(layout.numOfVertices * getAttributeCompCount(entry));
			for (int i = 0; i < layout.numOfVertices; i++)
			{
				glm::vec3 pos;
				for (int j = 0; j < getAttributeCompCount(entry); j++)
				{
					pos[j] = vertices[stride * i + j + offset];
				}
				positions.emplace_back(pos);
			}
			addPositions(positions);
		}

		// Parse normals
		else if (LayoutAttribute::Normals == entry)
		{
			normals.reserve(layout.numOfVertices * getAttributeCompCount(entry));
			for (int i = 0; i < layout.numOfVertices; i++)
			{
				glm::vec3 normal;
				for (int j = 0; j < getAttributeCompCount(entry); j++)
				{
					normal[j] = vertices[stride * i + j + offset];
				}
				normals.emplace_back(normal);
			}
			addNormals(normals);
		}

		// Parse texcoords
		else if (LayoutAttribute::Texcoords == entry)
		{
			texcoords.reserve(layout.numOfVertices * getAttributeCompCount(entry));
			for (int i = 0; i < layout.numOfVertices; i++)
			{
				glm::vec2 vec;
				for (int j = 0; j < getAttributeCompCount(entry); j++)
				{
					vec[j] = vertices[stride * i + j + offset];
				}
				texcoords.emplace_back(vec);
			}
			addTexcoords(texcoords);
		}

		// Parse colors
		else if (LayoutAttribute::Colors == entry)
		{
			colors.reserve(layout.numOfVertices * getAttributeCompCount(entry));
			for (int i = 0; i < layout.numOfVertices; i++)
			{
				glm::vec3 color;
				for (int j = 0; j < getAttributeCompCount(entry); j++)
				{
					color[j] = vertices[stride * i + j + offset];
				}
				colors.emplace_back(color);
			}
			addColors(colors);
		}

		// Parse Tangents
		else if (LayoutAttribute::Tangents == entry)
		{
			tangents.reserve(layout.numOfVertices * getAttributeCompCount(entry));
			for (int i = 0; i < layout.numOfVertices; i++)
			{
				glm::vec3 tangent;
				for (int j = 0; j < getAttributeCompCount(entry); j++)
				{
					tangent[j] = vertices[stride * i + j + offset];
				}
				colors.emplace_back(tangent);
			}
			addTangents(tangents);
		}

		offset += getAttributeCompCount(entry);
	}

	return *this;
}

MeshBuilder& MeshBuilder::setMaterialIndex(int index)
{
	m_data.materialIndex = index;

	return *this;
}



void MeshBuilder::build(Mesh& mesh)
{
	if (m_data.m_positions.size() > 0)
	{
		enableAttribute(LayoutAttribute::Positions);
	}
	if (m_data.m_normals.size() > 0)
	{
		enableAttribute(LayoutAttribute::Normals);
	}
	if (m_data.m_texCoords.size() > 0)
	{
		enableAttribute(LayoutAttribute::Texcoords);
	}
	if (m_data.m_colors.size() > 0)
	{
		enableAttribute(LayoutAttribute::Colors);
	}
	if (m_data.m_tangents.size() > 0)
	{
		enableAttribute(LayoutAttribute::Tangents);
	}
	if (m_data.bonesIDs.size() > 0)
	{
		enableAttribute(LayoutAttribute::BoneIDs);
	}
	if (m_data.bonesWeights.size() > 0)
	{
		enableAttribute(LayoutAttribute::BoneWeights);
	}

	std::sort(m_data.m_layout.attribs.begin(), m_data.m_layout.attribs.end(),
		[](LayoutAttribute l1, LayoutAttribute l2)
	{
		return getAttributeLocationInShader(l1) < getAttributeLocationInShader(l2);
	});

	//auto& projectDir = Engine::get()->getProjectDirectory();

	//MeshSerializer::writeDataToBinaryFile(m_data, projectDir + "/" + mesh.getUID() + ".bin");
	//Engine::get()->getContext()->getProjectAssetRegistry()->addMesh(mesh.getUID());

	//MeshData newMeshData;
	//MeshSerializer::readDataFromBinaryFile(mesh.getUID() + ".bin", newMeshData);

	if (!mesh.build(m_data))
	{
		logError("Mesh Builder failed to build mesh.");
	}
}

MeshBuilder& MeshBuilder::builder()
{
	return *new MeshBuilder();
}

MeshBuilder::MeshBuilder()
{
	
}

void MeshBuilder::enableAttribute(LayoutAttribute attribute)
{
	if (std::find(m_data.m_layout.attribs.begin(), m_data.m_layout.attribs.end(), attribute) == m_data.m_layout.attribs.end())
		m_data.m_layout.attribs.emplace_back(attribute);
}

void MeshBuilder::disableAttribute(LayoutAttribute attribute)
{
	auto iter = std::find(m_data.m_layout.attribs.begin(), m_data.m_layout.attribs.end(), attribute);
	if (iter != m_data.m_layout.attribs.end())
		m_data.m_layout.attribs.erase(iter);
}