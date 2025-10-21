#include "geometry/MeshCollection.h"

#include "geometry/ModelImporter.h"
#include "memory/AssetLoader.h"

#include "geometry/MeshExporter.h"

#include <filesystem>

namespace {
	struct MeshManagerRegistration {
		MeshManagerRegistration() {
			AssetFactory::registerManager(AssetType::MESH, std::make_shared<MeshCollectionAssetManager>());
		}
	} _meshManagerRegistration;
}

bool MeshCollectionAssetManager::copyFiles(const std::string& fileLocation, AssetInfo& aInfo)
{
	return Engine::get()->getSubSystem<ModelImporter>()->copyFiles(fileLocation, aInfo);
}

ResourceWrapper<ResourceBase> MeshCollectionAssetManager::load(AssetInfo& aInfo)
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
}

std::map<int, ResourceWrapper<Material>> MeshCollectionAssetManager::getLoadedMaterials()
{
	return m_lastLoadedModelInfo.materials;
}

void MeshCollectionAssetManager::save(const AssetWrapper<ResourceBase>& mesh, const AssetInfo& aInfo)
{
	MeshExporter::exportMesh(aInfo, mesh.as<MeshCollection>().resource());
}

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

AssetWrapper<MeshCollection> MeshCollection::import(const std::string& fileLocation, ModelImportSettings desc)
{
	desc.aType = AssetType::MESH;
	desc.assetDirectory = std::filesystem::path(fileLocation).filename().stem().generic_string();
	desc.origFilePath = fileLocation;
	return Engine::get()->getSubSystem<Assets>()->importAsset(fileLocation, desc).as<MeshCollection>();
}

std::map<int, ResourceWrapper<Material>> MeshCollection::getLastLoadedMaterials()
{
	return dynamic_cast<MeshCollectionAssetManager*>(AssetFactory::getManager(AssetType::MESH))->getLoadedMaterials(); // todo fix
}


