#include "geometry/Model.h"

#include "geometry/ModelImporter.h"

#include "core/Factory.h"
#include "core/Engine.h"
#include "geometry/MeshBuilder.h"
#include "memory/Assets.h"

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


ModelResourceRef Model::load(const std::string& fileLocation, ModelLoadDescriptor desc)
{
	ModelImporter::ModelInfo modelInfo;
	Engine::get()->getSubSystem<ModelImporter>()->parseModel(fileLocation, modelInfo);

	ModelResourceRef model = Factory<Model>::create();
	for (const auto& data : modelInfo.meshDataList)
	{
		MeshBuilder builder(data);
		auto mesh = builder.build();
		model->addMesh(mesh);
	}

	

	//ModelImporter::ModelInfo mInfo;
	//mInfo.mesh = mesh;
	//Engine::get()->getSubSystem<ModelImporter>()->loadModelFromFile(desc, mInfo);
	return model;
}

#include "geometry/MeshBinaryLoader.h"

void ModelAsset::bindDependency(const std::string& slot, UUID dependency)
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
		logWarning("ModelAsset: invalid SLOT format '{}'", slot);
		return;
	}

	int index = std::stoi(slot.substr(pos, end - pos));

	// Fetch the asset
	AssetRef<Asset> generic =
		Engine::get()->getSubSystem<Assets>()->getAsset(dependency);

	MaterialAssetRef material = generic.as<MaterialAsset>();

	if (material.isEmpty())
	{
		logWarning("ModelAsset: dependency '{}' is not a MaterialAsset", slot);
		return;
	}

	m_materials[index] = material;
}

void ModelAsset::serialize(nlohmann::json& j) const
{
	j["materials"] = m_materials;
}

void ModelAsset::deserialize(const nlohmann::json& j)
{
	j.at("materials").get_to(m_materials);
}