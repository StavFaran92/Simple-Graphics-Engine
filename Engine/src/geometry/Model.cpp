#include "geometry/Model.h"

#include "geometry/ModelImporter.h"

#include "geometry/MeshExporter.h"
#include "core/Factory.h"
#include "core/Engine.h"

#include <functional>

ResourceWrapper<Resource> ModeLoadDescriptor::loadResource()
{
	return Model::load(sourcePath, *this);
}

void Model::addMesh(const std::shared_ptr<Mesh>& mesh)
{
	m_materialSlots.insert(mesh->getMaterialIndex());

	m_meshes.push_back(mesh);
}

std::shared_ptr<Mesh> Model::getPrimaryMesh() const
{
	assert(m_meshes.size() > 0);
	return *m_meshes.begin();
}

std::vector<std::shared_ptr<Mesh>> Model::getMeshes() const
{
	return m_meshes;
}

size_t Model::getNumOfVertices() const
{
	size_t vCount = 0;
	for (auto& mesh : m_meshes)
	{
		vCount += mesh->getNumOfVertices();
	}
	return vCount;
}

void Model::addBonesInfo(const std::vector<glm::mat4>& bonesOffsets, const std::unordered_map<std::string, unsigned int>& bonesNameToIDMap)
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

std::vector<glm::mat4> Model::getBoneOffsets() const
{
	return m_bonesOffsets;
}

int Model::getBoneID(const std::string& boneName) const
{
	if (m_bonesNameToIDMap.find(boneName) == m_bonesNameToIDMap.end())
	{
		return -1;
	}

	return m_bonesNameToIDMap.at(boneName);
}

int Model::getMaterialCount() const
{
	return m_materialSlots.size();
}

const std::vector<AssetHandle<MaterialAsset>>& ModelAsset::getImportedMaterials() const
{
	return m_importedMaterials;
}

ResourceWrapper<Model> Model::load(const std::string& fileLocation, ModeLoadDescriptor desc)
{
	desc.sourcePath = fileLocation;
	ResourceWrapper<Model> mesh = Factory<Model>::create();
	ModelImporter::ModelInfo mInfo;
	mInfo.mesh = mesh;
	Engine::get()->getSubSystem<ModelImporter>()->loadModelFromFile(desc, mInfo);
	return mesh;
}