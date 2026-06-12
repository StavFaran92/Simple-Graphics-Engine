#include "geometry/MeshBuilder.h"

#include "core/Logger.h"
#include "render/VertexLayout.h"
#include "core/Factory.h"
#include "geometry/MeshSerializer.h"
#include "runtime/Context.h"
#include "serialize/ProjectAssetRegistry.h"
#include "utils/MikkTSpaceImpl.h"
#include "render/VAOManager.h"
#include "render/GigaVAO.h"
#include "geometry/StaticMesh.h"
#include "geometry/SkinnedMesh.h"

void MeshBuilder::addVertices(const std::vector<StaticVertex>& vertices)
{
	m_data.staticVertices.insert(m_data.staticVertices.end(), vertices.begin(), vertices.end());
}

void MeshBuilder::addVertices(const std::vector<SkinnedVertex>& vertices)
{
	m_data.skinnedVertices.insert(m_data.skinnedVertices.end(), vertices.begin(), vertices.end());
}

MeshBuilder& MeshBuilder::addIndex(unsigned int index)
{
	m_data.indices.push_back(index);

	return *this;
}

MeshBuilder& MeshBuilder::addIndices(const unsigned int* indices, size_t size)
{
	int offset = m_data.indices.size() > 0 ? m_data.indices[m_data.indices.size() - 1] + 1 : 0;
	for (int i = 0; i < size; i++)
	{
		addIndex(indices[i] + offset);
	}

	return *this;
}

MeshBuilder& MeshBuilder::addIndices(const std::vector<unsigned int>& indices)
{
	addIndices(&indices.data()[0], indices.size());

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
	std::vector<glm::vec4> tangents;

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

		// Parse Tangents
		else if (LayoutAttribute::Tangents == entry)
		{
			tangents.reserve(layout.numOfVertices * getAttributeCompCount(entry));
			for (int i = 0; i < layout.numOfVertices; i++)
			{
				glm::vec4 tangent;
				for (int j = 0; j < getAttributeCompCount(entry); j++)
				{
					tangent[j] = vertices[stride * i + j + offset];
				}
				tangents.emplace_back(tangent);
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

void GenerateTangentsForMesh(MeshData& mesh) {
	if (mesh.m_positions.empty() ||
		mesh.m_normals.empty() ||
		mesh.m_texCoords.empty() ||
		mesh.m_indices.empty()) {
		std::cerr << "Mesh missing necessary data for tangent generation." << std::endl;
		return;
	}

	// Ensure m_tangents is correctly sized
	mesh.m_tangents.resize(mesh.m_positions.size(), glm::vec4(0.0f));

	MikkMeshContext userData;
	userData.mesh = &mesh;

	SMikkTSpaceInterface iface{};
	iface.m_getNumFaces = getNumFaces;
	iface.m_getNumVerticesOfFace = getNumVertsOfFace;
	iface.m_getPosition = getPosition;
	iface.m_getNormal = getNormal;
	iface.m_getTexCoord = getTexCoord;
	iface.m_setTSpaceBasic = setTSpaceBasic;

	SMikkTSpaceContext context{};
	context.m_pInterface = &iface;
	context.m_pUserData = &userData;

	try
	{
		if (!genTangSpaceDefault(&context)) {
			logWarning("MikkTSpace tangent generation failed.");
		}
	}
	catch (std::exception e)
	{
		logWarning("MikkTSpace tangent generation failed.");
	}


	logDebug("Tagnents generation finished.");
}



std::shared_ptr<Mesh> MeshBuilder::build()
{
	std::shared_ptr<Mesh> mesh;

	GenerateTangentsForMesh(m_data);

	// validate mesh data
	if (m_data.getVertexCount() == 0)
	{
		logError("Cannot build mesh without vertices");
		return;
	}

	// calculate stride
	size_t stride = m_data.getStride();
	size_t numOfVertices = m_data.getVertexCount();

	if (m_data.getType() == MeshType::StaticMesh)
	{
		mesh = std::make_shared<StaticMesh>();

		auto& verts = m_data.staticVertices;

		auto gigaVAO = Engine::get()->getSubSystem<VAOManager>()->getGigaVAO(VAOManager::Type::StaticGeometry);
		unsigned int meshVertexOffset = 0;
		unsigned int meshIndexOffset = 0;
		gigaVAO.push(verts.data(), verts.size(), m_data.indices, meshVertexOffset, meshIndexOffset);

		glm::vec3 minAABB = glm::vec3(std::numeric_limits<float>::max());
		glm::vec3 maxAABB = glm::vec3(std::numeric_limits<float>::min());

		// TODO this can be optimized using assimp premade aabb structure
		for (auto&& v : verts)
		{
			auto pos = v.position;
			minAABB.x = std::min(minAABB.x, pos.x);
			minAABB.y = std::min(minAABB.y, pos.y);
			minAABB.z = std::min(minAABB.z, pos.z);

			maxAABB.x = std::max(maxAABB.x, pos.x);
			maxAABB.y = std::max(maxAABB.y, pos.y);
			maxAABB.z = std::max(maxAABB.z, pos.z);
		}

		mesh->m_aabb = AABB::createFromMinMax(minAABB, maxAABB);
	}
	else if (m_data.getType() == MeshType::SkinnedMesh)
	{
		mesh = std::make_shared<SkinnedMesh>();

		auto& verts = m_data.staticVertices;

		auto gigaVAO = Engine::get()->getSubSystem<VAOManager>()->getGigaVAO(VAOManager::Type::SkinnedGeometry);
		unsigned int meshVertexOffset = 0; 
		unsigned int meshIndexOffset = 0; 
		gigaVAO.push(verts.data(), verts.size(), m_data.indices, meshVertexOffset, meshIndexOffset);

		glm::vec3 minAABB = glm::vec3(std::numeric_limits<float>::max());
		glm::vec3 maxAABB = glm::vec3(std::numeric_limits<float>::min());

		// TODO this can be optimized using assimp premade aabb structure
		for (auto&& v : verts)
		{
			auto pos = v.position;
			minAABB.x = std::min(minAABB.x, pos.x);
			minAABB.y = std::min(minAABB.y, pos.y);
			minAABB.z = std::min(minAABB.z, pos.z);

			maxAABB.x = std::max(maxAABB.x, pos.x);
			maxAABB.y = std::max(maxAABB.y, pos.y);
			maxAABB.z = std::max(maxAABB.z, pos.z);
		}

		mesh->m_aabb = AABB::createFromMinMax(minAABB, maxAABB);
	}

	

	//// Create buffers
	//m_vao = std::make_shared<VertexArrayObject>();

	//if (mData.m_indices.size() > 0)
	//{
	//	m_ibo = std::make_shared<ElementBufferObject>((unsigned int*)&(mData.m_indices[0]), mData.m_indices.size());
	//}


	//auto vbo = VertexBufferObject::createRaw(&(vertices[0]), m_layout.numOfVertices, bufferSize, m_layout);

	//delete[] vertices;

	//m_meshData = mData;
	//m_vao->attachBuffer(vbo, m_ibo.get());
	//m_vao->setVertexCount(mData.m_positions.size());
	//m_vao->build();


	mesh->materialIndex = m_data.materialIndex;
	//m_normals = std::move(mData.m_normals);


	

	//auto& projectDir = Engine::get()->getProjectDirectory();

	//MeshSerializer::writeDataToBinaryFile(m_data, projectDir + "/" + mesh.getUID() + ".bin");
	//Engine::get()->getContext()->getProjectAssetRegistry()->addMesh(mesh.getUID());

	//MeshData newMeshData;
	//MeshSerializer::readDataFromBinaryFile(mesh.getUID() + ".bin", newMeshData);
}

MeshBuilder& MeshBuilder::builder()
{
	return *new MeshBuilder();
}

MeshBuilder::MeshBuilder()
{
	
}

MeshBuilder::MeshBuilder(const MeshData& meshData)
	: m_data(std::move(meshData))
{
}

//void MeshBuilder::enableAttribute(LayoutAttribute attribute)
//{
//	if (std::find(m_data.m_layout.attribs.begin(), m_data.m_layout.attribs.end(), attribute) == m_data.m_layout.attribs.end())
//		m_data.m_layout.attribs.emplace_back(attribute);
//}
//
//void MeshBuilder::disableAttribute(LayoutAttribute attribute)
//{
//	auto iter = std::find(m_data.m_layout.attribs.begin(), m_data.m_layout.attribs.end(), attribute);
//	if (iter != m_data.m_layout.attribs.end())
//		m_data.m_layout.attribs.erase(iter);
//}