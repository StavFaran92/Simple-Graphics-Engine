#include "geometry/MeshCollection.h"

#include "geometry/ModelImporter.h"
#include "memory/AssetLoader.h"

#include <filesystem>

template<>
struct AssetTraits<MeshCollection>
{
	static bool copyFiles(const std::string& fileLocation, const AssetInfo& aInfo)
	{
		return Engine::get()->getSubSystem<ModelImporter>()->copyFiles(fileLocation, aInfo);
	}

	static ResourceWrapper<MeshCollection> load(const AssetInfo& aInfo)
	{
		UUID uuid = aInfo.uuid;
		MeshCollection* meshPtr = new MeshCollection();
		Engine::get()->getMemoryPool().add(uuid, meshPtr);
		ResourceWrapper<MeshCollection> generatedMesh(uuid);
		ModelImporter::ModelInfo mInfo;
		mInfo.mesh = generatedMesh;
		Engine::get()->getSubSystem<ModelImporter>()->loadModelFromFile(aInfo, mInfo);
		m_lastLoadedModelInfo = mInfo;
		return generatedMesh;

		//return Engine::get()->getSubSystem<AnimationLoader>()->load(aInfo);
	}

	static std::map<int, ResourceWrapper<Material>> getLoadedMaterials()
	{
		return m_lastLoadedModelInfo.materials;
	}

private:
	static inline ModelImporter::ModelInfo m_lastLoadedModelInfo;
};

static AssetFnRegister<AssetType::MESH> textureAssetRegister(AssetTraits<MeshCollection>::load);

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

ResourceWrapper<MeshCollection> MeshCollection::import(const std::string& fileLocation, const ModelImportSettings& settings)
{
	AssetInfo aInfo(settings);
	aInfo.aType = AssetType::MESH;
	aInfo.assetDirectory = (std::filesystem::path(settings.assetDirectory) / aInfo.name).generic_string();
	return AssetLoader<MeshCollection>::import(fileLocation, aInfo);
}

ResourceWrapper<MeshCollection> MeshCollection::loadTransient(const std::string& fileLocation, const ModelImportSettings& settings)
{
	AssetInfo aInfo(settings);
	aInfo.aType = AssetType::MESH;
	aInfo.assetDirectory = (std::filesystem::path(settings.assetDirectory) / aInfo.name).generic_string();
	return AssetLoader<MeshCollection>::loadTransient(fileLocation, aInfo);
}

std::map<int, ResourceWrapper<Material>> MeshCollection::getLastLoadedMaterials()
{
	return AssetTraits<MeshCollection>::getLoadedMaterials();
}

//Resource<MeshCollection> MeshCollection::import(const std::string& fileLocation, const ModelImporter::ModelImportSettings& settings)
//{
//	return Resource<MeshCollection>();
//}

//Resource<MeshCollection> MeshCollection::loadInner(AssetInfo aInfo)
//{
//	UUID uuid = aInfo.uuid;
//	MeshCollection* meshPtr = new MeshCollection();
//	Engine::get()->getMemoryPool().add(uuid, meshPtr);
//	Resource<MeshCollection> generatedMesh(uuid);
//	ModelImporter::ModelInfo mInfo;
//	mInfo.mesh = generatedMesh;
//	Engine::get()->getResourceManager()->incRef(uuid);
//	const std::string filepath = Engine::get()->getProjectDirectory() + aInfo.filePath;
//	Engine::get()->getSubSystem<ModelImporter>()->loadModelFromFile(filepath, mInfo);
//	return generatedMesh;
//}
