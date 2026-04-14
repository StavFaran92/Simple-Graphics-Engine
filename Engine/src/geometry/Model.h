#pragma once

#include <vector>
#include "core/Core.h"
#include "geometry/Mesh.h"
#include "memory/ResourceWrapper.h"
#include "memory/Asset.h"
#include "geometry/ModelImporter.h"

struct ModelData
{
	std::vector<MeshData> m_meshes;
	std::vector<glm::mat4> m_bonesOffsets;
	std::unordered_map<std::string, unsigned int> m_bonesNameToIDMap;
	std::set<int> m_materialSlots{};
};

struct EngineAPI ModelLoadDescriptor : public ResourceLoadDescriptor
{
};

struct EngineAPI ModelCreateDescriptor : public ResourceBuildDescriptor
{
	ModelData data;
};

// Resource
class EngineAPI ModelResource : public Resource
{
public:
	static ResourceWrapper<Model> load(const std::string& fileLocation, ModelLoadDescriptor desc = {});

	void addMesh(const std::shared_ptr<Mesh>& mesh);

	std::shared_ptr<Mesh> getPrimaryMesh() const;

	std::vector<std::shared_ptr<Mesh>> getMeshes() const;

	size_t getNumOfVertices() const;

	void addBonesInfo(const std::vector<glm::mat4>& bonesOffsets, const std::unordered_map<std::string, unsigned int>& bonesNameToIDMap);

	std::vector<glm::mat4> getBoneOffsets() const;

	int getBoneID(const std::string& boneName) const;

	int getMaterialCount() const;
private:
	std::vector<std::shared_ptr<Mesh>> m_meshes;
	std::vector<glm::mat4> m_bonesOffsets;
	std::unordered_map<std::string, unsigned int> m_bonesNameToIDMap;
	std::set<int> m_materialSlots{};
};

// Asset
class EngineAPI ModelAsset : public Asset
{
public:
	ModelAsset() = default;

	void bindDependency(const std::string& slot, UUID dependency) override;

	std::map<int, AssetHandle<MaterialAsset>> m_materials;

	// Inherited via Asset
	void serialize(nlohmann::json& j) const override;
	void deserialize(const nlohmann::json& j) override;
};

struct Model
{
	using AssetType = ModelAsset;
	using ResourceType = ModelResource;
};

CEREAL_REGISTER_TYPE(ModelAsset);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Asset, ModelAsset)