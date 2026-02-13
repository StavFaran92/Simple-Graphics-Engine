#include "geometry/MeshGroup.h"

#include "geometry/ModelImporter.h"

#include "geometry/MeshExporter.h"
#include "core/Factory.h"
#include "core/Engine.h"

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

AssetHandle<MeshGroupAsset> MeshGroupAsset::import(const std::string& fileLocation, AssetCreateDescriptor desc)
{
	desc.aType = AssetType::MESH;
	desc.isCompositeAsset = true;
	if (!desc.resourceDescriptor)
	{
		desc.makeResourceDescriptor<MeshGroupLoadDescriptor>();
	}
	MeshGroupAsset* asset = new MeshGroupAsset(desc);
	return asset->importAsset(fileLocation).as<MeshGroupAsset>();
}

AssetHandle<MeshGroupAsset> MeshGroupAsset::create(const ResourceWrapper<MeshGroup>& mesh, AssetCreateDescriptor desc)
{
	desc.aType = AssetType::MESH;
	MeshGroupAsset* asset = new MeshGroupAsset(desc);
	return asset->createAsset(mesh).as<MeshGroupAsset>();
}

const std::vector<AssetHandle<MaterialAsset>>& MeshGroupAsset::getImportedMaterials() const
{
	return m_importedMaterials;
}

bool MeshGroupAsset::copyFiles(const std::string& fileLocation, AssetRecord& aInfo)
{
	return Engine::get()->getSubSystem<ModelImporter>()->copyFiles(fileLocation, aInfo);
}

void MeshGroupAsset::save(const AssetRecord& aInfo)
{
	MeshExporter::exportMesh(AssetHandle<MeshGroupAsset>(m_uuid));
}

ResourceWrapper<MeshGroup> MeshGroup::load(const std::string& fileLocation, MeshGroupLoadDescriptor desc)
{
	desc.sourcePath = fileLocation;
	ResourceWrapper<MeshGroup> mesh = Factory<MeshGroup>::create();
	ModelImporter::ModelInfo mInfo;
	mInfo.mesh = mesh;
	Engine::get()->getSubSystem<ModelImporter>()->loadModelFromFile(desc, mInfo);
	return mesh;
}


ResourceWrapper<Resource> MeshGroupLoadDescriptor::loadResource() {
	return MeshGroup::load(sourcePath, *this);
}