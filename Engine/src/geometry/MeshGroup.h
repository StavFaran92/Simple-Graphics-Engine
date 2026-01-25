#pragma once

#include <vector>
#include "core/Core.h"
#include "geometry/Mesh.h"
#include "memory/ResourceWrapper.h"
#include "memory/Asset.h"
#include "geometry/ModelImporter.h"

// Asset IO Manager
struct MeshGroupAssetManager : public AssetManager
{
	bool copyFiles(const std::string& fileLocation, AssetRecord& aInfo) override;
	ResourceWrapper<Resource> load(AssetRecord& aInfo) override;
	void save(AssetHandle<Asset> asset, const AssetRecord& aInfo) override;
};

// Resource
class EngineAPI MeshGroup : public Resource
{
public:
	static ResourceWrapper<MeshGroup> load(const std::string& fileLocation, ModelImportSettings aDesc = {});

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
class EngineAPI MeshGroupAsset : public Asset
{
public:
	using ResourceType = MeshGroup;
	static AssetHandle<MeshGroupAsset> import(const std::string& fileLocation, ModelImportSettings aDesc = {});

	const std::vector<AssetHandle<MaterialAsset>>& getImportedMaterials() const;

private:
	std::vector<AssetHandle<MaterialAsset>> m_importedMaterials;
};