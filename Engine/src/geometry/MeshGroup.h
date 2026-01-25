#pragma once

#include <vector>
#include "core/Core.h"
#include "geometry/Mesh.h"
#include "memory/ResourceWrapper.h"
#include "memory/Asset.h"
#include "geometry/ModelImporter.h"

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

	using Asset::Asset;

	static AssetHandle<MeshGroupAsset> import(const std::string& fileLocation, ModelImportSettings aDesc = {});

	const std::vector<AssetHandle<MaterialAsset>>& getImportedMaterials() const;

	void save(const AssetRecord& aInfo) override;
protected:
	bool copyFiles(const std::string& fileLocation, AssetRecord&) override;


private:
	std::vector<AssetHandle<MaterialAsset>> m_importedMaterials;
};