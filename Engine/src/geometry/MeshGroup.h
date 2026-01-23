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
	bool copyFiles(const std::string& fileLocation, AssetInfo& aInfo) override;
	ResourceWrapper<Resource> load(AssetInfo& aInfo) override;
	std::map<int, AssetWrapper<Material>> getLoadedMaterials();
	void save(Asset meshGroup, const AssetInfo& aInfo) override;

private:
	ModelImporter::ModelInfo m_lastLoadedModelInfo;
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
	using Asset::Asset;

	MeshGroupAsset(const Asset& asset);

	static MeshGroupAsset import(const std::string& fileLocation, ModelImportSettings aDesc = {});

	static std::map<int, AssetWrapper<Material>> getLastLoadedMaterials();

	ResourceWrapper<MeshGroup> resource() const;
	ResourceWrapper<MeshGroup> resource();
};