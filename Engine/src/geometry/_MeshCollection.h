#pragma once

#include <vector>

#include "core/Core.h"
#include "geometry/Mesh.h"
#include "memory/ResourceWrapper.h"
#include "memory/Asset.h"
#include "geometry/ModelImporter.h"

struct MeshGroupAssetManager : public AssetManager
{
	bool copyFiles(const std::string& fileLocation, AssetRecord& aInfo) override;
	ResourceWrapper<Resource> load(AssetRecord& aInfo) override;
	std::map<int, AssetHandle<Material>> getLoadedMaterials();
	void save(const AssetHandle<Resource>& mat, const AssetRecord& aInfo) override;

private:
	ModelImporter::ModelInfo m_lastLoadedModelInfo;
};

class EngineAPI MeshGroup : public Resource
{
public:
	void addMesh(const std::shared_ptr<Mesh>& mesh);

	std::shared_ptr<Mesh> getPrimaryMesh() const;

	std::vector<std::shared_ptr<Mesh>> getMeshes() const;

	size_t getNumOfVertices() const;

	void addBonesInfo(const std::vector<glm::mat4>& bonesOffsets, const std::unordered_map<std::string, unsigned int>& bonesNameToIDMap);

	std::vector<glm::mat4> getBoneOffsets() const;

	int getBoneID(const std::string& boneName) const;

	int getMaterialCount() const;

	static AssetHandle<MeshGroupAsset> import(const std::string& fileLocation, ModelImportSettings aDesc = {});
	static ResourceWrapper<MeshGroup> load(const std::string& fileLocation, ModelImportSettings aDesc = {});

	static std::map<int, AssetHandle<Material>> getLastLoadedMaterials();
	

private:
	std::vector<std::shared_ptr<Mesh>> m_meshes;
	std::vector<glm::mat4> m_bonesOffsets;
	std::unordered_map<std::string, unsigned int> m_bonesNameToIDMap;
	std::set<int> m_materialSlots{};
};