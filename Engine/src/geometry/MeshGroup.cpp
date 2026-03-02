#include "geometry/MeshGroup.h"

#include "geometry/ModelImporter.h"

#include "geometry/MeshExporter.h"
#include "core/Factory.h"
#include "core/Engine.h"
#include "geometry/MeshBuilder.h"

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


ResourceWrapper<MeshGroup> MeshGroup::load(const std::string& fileLocation, MeshGroupLoadDescriptor desc)
{
	ModelImporter::ModelInfo modelInfo;
	Engine::get()->getSubSystem<ModelImporter>()->parseModel(fileLocation, modelInfo);

	ResourceWrapper<MeshGroup> model = Factory<MeshGroup>::create();
	for (const auto& data : modelInfo.meshDataList)
	{
		std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>();
		MeshBuilder builder(data);
		builder.build(*mesh.get());
		model->addMesh(mesh);
	}

	

	//ModelImporter::ModelInfo mInfo;
	//mInfo.mesh = mesh;
	//Engine::get()->getSubSystem<ModelImporter>()->loadModelFromFile(desc, mInfo);
	return model;
}

#include "geometry/MeshBinaryLoader.h"
ResourceWrapper<Resource> MeshGroupLoadDescriptor::loadResource() {
	std::vector<MeshData> meshDataList;
	MeshBinaryLoader::load(sourcePath, meshDataList);

	ResourceWrapper<MeshGroup> model = Factory<MeshGroup>::create();
	for (const auto& data : meshDataList)
	{
		std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>();
		MeshBuilder builder(data);
		builder.build(*mesh.get());
		model->addMesh(mesh);
	}

	return model;
}

ResourceWrapper<Resource> MeshGroupCreateDescriptor::createResource()
{
	throw std::exception("Not yet implmeneted.");
	//return MeshGroup::cre(sourcePath, *this);
}

void MeshGroupAsset::bindDependency(const std::string& slot, UUID dependency)
{
	// We only care about materials here
	const std::string token = "SLOT_";

	size_t pos = slot.find(token);
	if (pos == std::string::npos)
		return; // not a material slot binding

	pos += token.size();

	// Extract consecutive digits after SLOT_
	size_t end = pos;
	while (end < slot.size() && std::isdigit(slot[end]))
		++end;

	if (end == pos)
	{
		logWarning("MeshGroupAsset: invalid SLOT format '{}'", slot);
		return;
	}

	int index = std::stoi(slot.substr(pos, end - pos));

	// Fetch the asset
	AssetHandle<Asset> generic =
		Engine::get()->getSubSystem<Assets>()->getAsset(dependency);

	AssetHandle<MaterialAsset> material = generic.as<MaterialAsset>();

	if (material.isEmpty())
	{
		logWarning("MeshGroupAsset: dependency '{}' is not a MaterialAsset", slot);
		return;
	}

	m_materials[index] = material;
}
