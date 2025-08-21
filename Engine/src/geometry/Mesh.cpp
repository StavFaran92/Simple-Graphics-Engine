#include "geometry/Mesh.h"

#include "geometry/MeshBuilder.h"
#include "core/Logger.h"
#include "render/VertexArrayObject.h"

#include <GL\glew.h>

Mesh::Mesh()
{
}

const std::vector<glm::vec3>& Mesh::getPositions() const
{
	return m_positions;
}

const std::vector<glm::vec3>& Mesh::getNormals() const
{
	return m_normals;
}

size_t Mesh::getNumOfVertices() const
{
	return m_positions.size();
}

bool Mesh::build(MeshData& mData)
{
	// validate mesh data
	if (mData.m_positions.size() == 0)
	{
		logError("Cannot build mesh without position data.");
		return false;
	}

	if (mData.m_layout.attribs.size() == 0)
	{
		logError("Number of attributes cannot be 0.");
		return false;
	}

	// if mesh doesn't have normals calculate them
	if (mData.m_normals.size() == 0)
	{
		calculateNormals();
	}

	m_layout = mData.m_layout;

	// calculate stride
	int stride = 0;
	for (auto entry : m_layout.attribs)
	{
		auto& attribData = getAttributeData(entry);
		stride += attribData.length * attribData.size;
	}

	// Update layout info
	m_layout.numOfVertices = mData.m_positions.size();
	m_layout.stride = stride;

	// Create verticies array
	// array size = size of each attribute * size of elements in attribute * vertices count
	int offset = 0;
	unsigned int bufferSize = stride * m_layout.numOfVertices;
	unsigned char* vertices = new unsigned char[bufferSize];

	for (auto entry : m_layout.attribs)
	{
		auto& attribData = getAttributeData(entry);

		// Parse positions
		if (LayoutAttribute::Positions == entry)
		{
			for (int i = 0; i < m_layout.numOfVertices; i++)
			{
				auto pos = mData.m_positions.at(i);
				auto vOffset = stride * i + offset;
				memcpy(vertices + vOffset + attribData.size * 0, &pos.x, attribData.size);
				memcpy(vertices + vOffset + attribData.size * 1, &pos.y, attribData.size);
				memcpy(vertices + vOffset + attribData.size * 2, &pos.z, attribData.size);
			}
		}

		// Parse normals
		else if (LayoutAttribute::Normals == entry)
		{
			for (int i = 0; i < m_layout.numOfVertices; i++)
			{
				auto normal = mData.m_normals.at(i);
				auto vOffset = stride * i + offset;
				memcpy(vertices + vOffset + attribData.size * 0, &normal.x, attribData.size);
				memcpy(vertices + vOffset + attribData.size * 1, &normal.y, attribData.size);
				memcpy(vertices + vOffset + attribData.size * 2, &normal.z, attribData.size);
			}
		}

		// Parse texcoords
		else if (LayoutAttribute::Texcoords == entry)
		{
			for (int i = 0; i < m_layout.numOfVertices; i++)
			{
				auto texCoord = mData.m_texCoords.at(i);
				auto vOffset = stride * i + offset;
				memcpy(vertices + vOffset + attribData.size * 0, &texCoord.x, attribData.size);
				memcpy(vertices + vOffset + attribData.size * 1, &texCoord.y, attribData.size);
			}
		}

		// Parse colors
		else if (LayoutAttribute::Colors == entry)
		{
			for (int i = 0; i < m_layout.numOfVertices; i++)
			{
				auto color = mData.m_colors.at(i);
				auto vOffset = stride * i + offset;
				memcpy(vertices + vOffset + attribData.size * 0, &color.x, attribData.size);
				memcpy(vertices + vOffset + attribData.size * 1, &color.y, attribData.size);
				memcpy(vertices + vOffset + attribData.size * 2, &color.z, attribData.size);
			}
		}

		// Parse tangents
		else if (LayoutAttribute::Tangents == entry)
		{
			for (int i = 0; i < m_layout.numOfVertices; i++)
			{
				auto tangent = mData.m_tangents.at(i);
				auto vOffset = stride * i + offset;
				memcpy(vertices + vOffset + attribData.size * 0, &tangent.x, attribData.size);
				memcpy(vertices + vOffset + attribData.size * 1, &tangent.y, attribData.size);
			}
		}

		// Parse tangents
		else if (LayoutAttribute::BoneIDs == entry)
		{
			for (int i = 0; i < m_layout.numOfVertices; i++)
			{
				auto boneIDs = mData.bonesIDs.at(i);
				auto vOffset = stride * i + offset;
				memcpy(vertices + vOffset + attribData.size * 0, &boneIDs.x, attribData.size);
				memcpy(vertices + vOffset + attribData.size * 1, &boneIDs.y, attribData.size);
				memcpy(vertices + vOffset + attribData.size * 2, &boneIDs.z, attribData.size);
			}
		}

		// Parse tangents
		else if (LayoutAttribute::BoneWeights == entry)
		{
			for (int i = 0; i < m_layout.numOfVertices; i++)
			{
				auto boneWeights = mData.bonesWeights.at(i);
				auto vOffset = stride * i + offset;
				memcpy(vertices + vOffset + attribData.size * 0, &boneWeights.x, attribData.size);
				memcpy(vertices + vOffset + attribData.size * 1, &boneWeights.y, attribData.size);
				memcpy(vertices + vOffset + attribData.size * 2, &boneWeights.z, attribData.size);
			}
		}

		
		offset += attribData.length * attribData.size;
	}

	// Create buffers
	m_vao = std::make_shared<VertexArrayObject>();

	if (mData.m_indices.size() > 0)
	{
		m_ibo = std::make_shared<ElementBufferObject>((unsigned int*)&(mData.m_indices[0]), mData.m_indices.size());
	}


	auto vbo = VertexBufferObject::createRaw(&(vertices[0]), m_layout.numOfVertices, bufferSize, m_layout);

	delete[] vertices;

	m_positions = mData.m_positions;
	m_vao->attachBuffer(vbo, m_ibo.get());
	m_vao->setVertexCount(m_positions.size());
	m_vao->build();


	glm::vec3 minAABB = glm::vec3(std::numeric_limits<float>::max());
	glm::vec3 maxAABB = glm::vec3(std::numeric_limits<float>::min());

	// TODO this can be optimized using assimp premade aabb structure
	for (auto&& pos : m_positions)
	{
		minAABB.x = std::min(minAABB.x, pos.x);
		minAABB.y = std::min(minAABB.y, pos.y);
		minAABB.z = std::min(minAABB.z, pos.z);

		maxAABB.x = std::max(maxAABB.x, pos.x);
		maxAABB.y = std::max(maxAABB.y, pos.y);
		maxAABB.z = std::max(maxAABB.z, pos.z);
	}

	m_aabb = AABB::createFromMinMax(minAABB, maxAABB);
	materialIndex = mData.materialIndex;
	//m_normals = std::move(mData.m_normals);

	return true;
}

// Compute the normals of the mesh
void Mesh::calculateNormals()
{
	//m_normals = std::vector<glm::vec3>(getNbVertices(), Vector3(0, 0, 0));

	//// For each triangular face
	//for (uint i = 0; i < getNbFaces(); i++) {

	//	// Get the three vertices index of the current face
	//	uint v1 = getVertexIndexInFace(i, 0);
	//	uint v2 = getVertexIndexInFace(i, 1);
	//	uint v3 = getVertexIndexInFace(i, 2);

	//	assert(v1 < getNbVertices());
	//	assert(v2 < getNbVertices());
	//	assert(v3 < getNbVertices());

	//	// Compute the normal of the face
	//	Vector3 p = getVertex(v1);
	//	Vector3 q = getVertex(v2);
	//	Vector3 r = getVertex(v3);
	//	Vector3 normal = (q - p).cross(r - p).normalize();

	//	// Add the face surface normal to the sum of normals at
	//	// each vertex of the face
	//	mNormals[v1] += normal;
	//	mNormals[v2] += normal;
	//	mNormals[v3] += normal;
	//}

	//// Normalize the normal at each vertex
	//for (uint i = 0; i < getNbVertices(); i++) {
	//	mNormals[i] = mNormals[i].normalize();
	//}
}

Resource<Asset> Mesh::import(const std::string& fileLocation, const ImportSettings& settings)
{
	return {};
}

Resource<Asset> Mesh::load(AssetInfo aInfo)
{
	return {};
}

void Mesh::clearMesh()
{
	m_positions.clear();
	m_normals.clear();
	m_vao = nullptr;
	m_ibo = nullptr;
	m_vbo = nullptr;
}

void Mesh::setVertexLayout(VertexLayout layout)
{
	m_layout = layout;
}

VertexLayout Mesh::getVertexLayout()
{
	return m_layout;
}

VertexArrayObject* Mesh::getVAO() const
{
	return m_vao.get();
}

AABB Mesh::getAABB() const
{
	return m_aabb;
}

int Mesh::getMaterialIndex() const
{
	return materialIndex;
}

Mesh::~Mesh()
{
	//logDebug( __FUNCTION__ );
	clearMesh();
}