#include "geometry/MeshGroup.h"

#include "geometry/ModelImporter.h"

#include "geometry/MeshExporter.h"
#include "core/Factory.h"

namespace {
	struct MeshManagerRegistration {
		MeshManagerRegistration() {
			AssetFactory::registerManager(AssetType::MESH, std::make_shared<MeshGroupAssetManager>());
		}
	} _meshManagerRegistration;
}

bool MeshGroupAssetManager::copyFiles(const std::string& fileLocation, AssetRecord& aInfo)
{
	return Engine::get()->getSubSystem<ModelImporter>()->copyFiles(fileLocation, aInfo);
}

ResourceWrapper<Resource> MeshGroupAssetManager::load(AssetRecord& aInfo)
{
	ResourceWrapper<MeshGroup> mesh = Factory<MeshGroup>::create();
	ModelImporter::ModelInfo mInfo;
	mInfo.mesh = mesh;
	Engine::get()->getSubSystem<ModelImporter>()->loadModelFromFile(aInfo, mInfo);
	return mesh;
}

void MeshGroupAssetManager::save(AssetHandle<Asset> mesh, const AssetRecord& aInfo)
{
	MeshExporter::exportMesh(mesh.as<MeshGroupAsset>());
}

void MeshGroup::addMesh(const std::shared_ptr<Mesh>& mesh)
{
	m_materialSlots.insert(mesh->getMaterialIndex());

	m_meshes.push_back(mesh);
}

std::shared_ptr<Mesh> MeshGroup::getPrimaryMesh() const
{
	assert(m_meshes.size() > 0);
	return *m_meshes.begin();
}

std::vector<std::shared_ptr<Mesh>> MeshGroup::getMeshes() const
{
	return m_meshes;
}

size_t MeshGroup::getNumOfVertices() const
{
	size_t vCount = 0;
	for (auto& mesh : m_meshes)
	{
		vCount += mesh->getNumOfVertices();
	}
	return vCount;
}

void MeshGroup::addBonesInfo(const std::vector<glm::mat4>& bonesOffsets, const std::unordered_map<std::string, unsigned int>& bonesNameToIDMap)
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

std::vector<glm::mat4> MeshGroup::getBoneOffsets() const
{
	return m_bonesOffsets;
}

int MeshGroup::getBoneID(const std::string& boneName) const
{
	if (m_bonesNameToIDMap.find(boneName) == m_bonesNameToIDMap.end())
	{
		return -1;
	}

	return m_bonesNameToIDMap.at(boneName);
}

int MeshGroup::getMaterialCount() const
{
	return m_materialSlots.size();
}

//MeshGroupAsset::MeshGroupAsset(const Asset& asset) : Asset(asset)
//{
//}

AssetHandle<MeshGroupAsset> MeshGroupAsset::import(const std::string& fileLocation, ModelImportSettings desc)
{
	desc.aType = AssetType::MESH;
	desc.isCompositeAsset = true;
	return Engine::get()->getSubSystem<Assets>()->importAsset(fileLocation, desc).as<MeshGroupAsset>();
}

const std::vector<AssetHandle<MaterialAsset>>& MeshGroupAsset::getImportedMaterials() const
{
	return m_importedMaterials;
}

//ResourceWrapper<MeshGroup> MeshGroupAsset::resource() const
//{
//	return resourceInner().as<MeshGroup>();
//}

ResourceWrapper<MeshGroup> MeshGroup::load(const std::string& fileLocation, ModelImportSettings aDesc)
{
	aDesc.aType = AssetType::MESH;
	aDesc.origFilePath = fileLocation;
	return Engine::get()->getSubSystem<Assets>()->loadResource(fileLocation, aDesc).as<MeshGroup>();
}


