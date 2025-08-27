#include "geometry/MeshCollection.h"

#include "geometry/ModelImporter.h"

static AssetFnRegister<AssetType::MESH> textureAssetRegister(MeshCollection::loadInner);

void MeshCollection::addMesh(const std::shared_ptr<Mesh>& mesh)
{
	m_meshes.push_back(mesh);
}

std::shared_ptr<Mesh> MeshCollection::getPrimaryMesh() const
{
	assert(m_meshes.size() > 0);
	return *m_meshes.begin();
}

std::vector<std::shared_ptr<Mesh>> MeshCollection::getMeshes() const
{
	return m_meshes;
}

size_t MeshCollection::getNumOfVertices() const
{
	size_t vCount = 0;
	for (auto& mesh : m_meshes)
	{
		vCount += mesh->getNumOfVertices();
	}
	return vCount;
}

void MeshCollection::addBonesInfo(const std::vector<glm::mat4>& bonesOffsets, const std::unordered_map<std::string, unsigned int>& bonesNameToIDMap)
{
	for (const glm::mat4& offset : bonesOffsets)
	{
		m_bonesOffsets.push_back(offset);
	}
	for (auto nameToID : bonesNameToIDMap)
	{
		m_bonesNameToIDMap.emplace(nameToID);
	}
}

std::vector<glm::mat4> MeshCollection::getBoneOffsets() const
{
	return m_bonesOffsets;
}

int MeshCollection::getBoneID(const std::string& boneName) const
{
	if (m_bonesNameToIDMap.find(boneName) == m_bonesNameToIDMap.end())
	{
		return -1;
	}

	return m_bonesNameToIDMap.at(boneName);
}

//Resource<MeshCollection> MeshCollection::import(const std::string& fileLocation, const ModelImporter::ModelImportSettings& settings)
//{
//	return Resource<MeshCollection>();
//}

Resource<MeshCollection> MeshCollection::loadInner(AssetInfo aInfo)
{
	UUID uuid = aInfo.uuid;
	MeshCollection* meshPtr = new MeshCollection();
	Engine::get()->getMemoryPool().add(uuid, meshPtr);
	Resource<MeshCollection> generatedMesh(uuid);
	ModelImporter::ModelInfo mInfo;
	mInfo.mesh = generatedMesh;
	Engine::get()->getResourceManager()->incRef(uuid);
	const std::string filepath = Engine::get()->getProjectDirectory() + aInfo.filePath;
	Engine::get()->getSubSystem<ModelImporter>()->loadModelFromFile(filepath, mInfo);
	return generatedMesh;
}
