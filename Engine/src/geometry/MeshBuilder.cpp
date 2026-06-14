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

MeshBuilder& MeshBuilder::addVertices(const std::vector<VertexVariant>& vertices)
{
	m_data.vertices.insert(m_data.vertices.end(), vertices.begin(), vertices.end());

	return *this;
}

MeshBuilder& MeshBuilder::addVertex(VertexVariant vertex)
{
	m_data.vertices.push_back(vertex);

	return *this;
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

MeshBuilder& MeshBuilder::addRawVertices(
	const float* vertices,
	VertexLayout layout)
{
	if (!vertices)
	{
		logError("Specified ptr is null.");
		return *this;
	}

	const uint8_t* data = reinterpret_cast<const uint8_t*>(vertices);

	for (size_t i = 0; i < layout.numOfVertices; ++i)
	{
		const uint8_t* src = data + i * layout.getStride();

		if (m_data.type == MeshType::StaticMesh)
		{
			StaticVertex vertex{};

			size_t offset = 0;

			for (LayoutAttribute attr : layout.attribs)
			{
				const auto& info = getAttributeData(attr);
				const uint8_t* ptr = src + offset;

				switch (attr)
				{
				case LayoutAttribute::Positions:
					memcpy(&vertex.position, ptr, sizeof(vertex.position));
					break;

				case LayoutAttribute::Normals:
					memcpy(&vertex.normal, ptr, sizeof(vertex.normal));
					break;

				case LayoutAttribute::Texcoords:
					memcpy(&vertex.texCoord, ptr, sizeof(vertex.texCoord));
					break;

				case LayoutAttribute::Tangents:
					memcpy(&vertex.tangent, ptr, sizeof(vertex.tangent));
					break;

				default:
					break;
				}

				offset += info.length * info.size;
			}

			m_data.vertices.push_back(vertex);
		}
		else
		{
			SkinnedVertex vertex{};

			size_t offset = 0;

			for (LayoutAttribute attr : layout.attribs)
			{
				const auto& info = getAttributeData(attr);
				const uint8_t* ptr = src + offset;

				switch (attr)
				{
				case LayoutAttribute::Positions:
					memcpy(&vertex.position, ptr, sizeof(vertex.position));
					break;

				case LayoutAttribute::Normals:
					memcpy(&vertex.normal, ptr, sizeof(vertex.normal));
					break;

				case LayoutAttribute::Texcoords:
					memcpy(&vertex.texCoord, ptr, sizeof(vertex.texCoord));
					break;

				case LayoutAttribute::Tangents:
					memcpy(&vertex.tangent, ptr, sizeof(vertex.tangent));
					break;

				case LayoutAttribute::BoneIDs:
					memcpy(&vertex.bonesIDs, ptr, sizeof(vertex.bonesIDs));
					break;

				case LayoutAttribute::BoneWeights:
					memcpy(&vertex.bonesWeights, ptr, sizeof(vertex.bonesWeights));
					break;

				default:
					break;
				}

				offset += info.length * info.size;
			}

			m_data.vertices.push_back(vertex);
		}
	}

	return *this;
}

MeshBuilder& MeshBuilder::setMaterialIndex(int index)
{
	m_data.materialIndex = index;

	return *this;
}

void GenerateTangentsForMesh(MeshData& mesh) {
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
		return nullptr;
	}

	// calculate stride
	size_t stride = m_data.getStride();
	size_t numOfVertices = m_data.getVertexCount();

	if (m_data.type == MeshType::StaticMesh)
	{
		mesh = std::make_shared<StaticMesh>();

		auto& verts = m_data.vertices;

		auto gigaVAO = Engine::get()->getSubSystem<VAOManager>()->getGigaVAO(VAOManager::Type::StaticGeometry);
		unsigned int meshVertexOffset = 0;
		unsigned int meshIndexOffset = 0;
		gigaVAO.push(verts.data(), verts.size(), m_data.indices, meshVertexOffset, meshIndexOffset);

		glm::vec3 minAABB = glm::vec3(std::numeric_limits<float>::max());
		glm::vec3 maxAABB = glm::vec3(std::numeric_limits<float>::min());

		// TODO this can be optimized using assimp premade aabb structure
		for (auto&& v : verts)
		{
			auto skinnedVertex = std::get<StaticVertex>(v);
			auto pos = skinnedVertex.position;
			minAABB.x = std::min(minAABB.x, pos.x);
			minAABB.y = std::min(minAABB.y, pos.y);
			minAABB.z = std::min(minAABB.z, pos.z);

			maxAABB.x = std::max(maxAABB.x, pos.x);
			maxAABB.y = std::max(maxAABB.y, pos.y);
			maxAABB.z = std::max(maxAABB.z, pos.z);
		}

		mesh->m_aabb = AABB::createFromMinMax(minAABB, maxAABB);
	}
	else if (m_data.type == MeshType::SkinnedMesh)
	{
		mesh = std::make_shared<SkinnedMesh>();

		auto& verts = m_data.vertices;

		auto gigaVAO = Engine::get()->getSubSystem<VAOManager>()->getGigaVAO(VAOManager::Type::SkinnedGeometry);
		unsigned int meshVertexOffset = 0; 
		unsigned int meshIndexOffset = 0; 
		gigaVAO.push(verts.data(), verts.size(), m_data.indices, meshVertexOffset, meshIndexOffset);

		glm::vec3 minAABB = glm::vec3(std::numeric_limits<float>::max());
		glm::vec3 maxAABB = glm::vec3(std::numeric_limits<float>::min());

		// TODO this can be optimized using assimp premade aabb structure
		for (auto&& v : verts)
		{
			auto skinnedVertex = std::get<SkinnedVertex>(v);
			auto pos = skinnedVertex.position;
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
	mesh->m_meshData = m_data;
	//m_normals = std::move(mData.m_normals);


	

	//auto& projectDir = Engine::get()->getProjectDirectory();

	//MeshSerializer::writeDataToBinaryFile(m_data, projectDir + "/" + mesh.getUID() + ".bin");
	//Engine::get()->getContext()->getProjectAssetRegistry()->addMesh(mesh.getUID());

	//MeshData newMeshData;
	//MeshSerializer::readDataFromBinaryFile(mesh.getUID() + ".bin", newMeshData);

	return mesh;
}

MeshBuilder::MeshBuilder(MeshType meshType)
{
	//todo move to const configs
	if (meshType == MeshType::StaticMesh)
	{
		m_data.m_layout.attribs.push_back(LayoutAttribute::Positions);
		m_data.m_layout.attribs.push_back(LayoutAttribute::Normals);
		m_data.m_layout.attribs.push_back(LayoutAttribute::Texcoords);
		m_data.m_layout.attribs.push_back(LayoutAttribute::Tangents);
	}
	else if (meshType == MeshType::SkinnedMesh)
	{
		m_data.m_layout.attribs.push_back(LayoutAttribute::Positions);
		m_data.m_layout.attribs.push_back(LayoutAttribute::Normals);
		m_data.m_layout.attribs.push_back(LayoutAttribute::Texcoords);
		m_data.m_layout.attribs.push_back(LayoutAttribute::Tangents);
		m_data.m_layout.attribs.push_back(LayoutAttribute::BoneIDs);
		m_data.m_layout.attribs.push_back(LayoutAttribute::BoneWeights);
	}
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