#pragma once

#include <vector>

#include "core/Core.h"
#include "geometry/Mesh.h"
#include "memory/ResourceWrapper.h"
#include "memory/Asset.h"
#include "geometry/ModelImporter.h"

class EngineAPI MeshCollection : public ResourceBase
{
public:
	void addMesh(const std::shared_ptr<Mesh>& mesh);

	std::shared_ptr<Mesh> getPrimaryMesh() const;

	std::vector<std::shared_ptr<Mesh>> getMeshes() const;

	size_t getNumOfVertices() const;

	void addBonesInfo(const std::vector<glm::mat4>& bonesOffsets, const std::unordered_map<std::string, unsigned int>& bonesNameToIDMap);

	std::vector<glm::mat4> getBoneOffsets() const;

	int getBoneID(const std::string& boneName) const;

	static ResourceWrapper<MeshCollection> import(const std::string& fileLocation, const ModelImportSettings& settings = {});

	static ResourceWrapper<MeshCollection> loadTransient(const std::string& fileLocation, const ModelImportSettings& settings = {});

	static std::map<int, ResourceWrapper<Material>> getLastLoadedMaterials();
	

private:
	std::vector<std::shared_ptr<Mesh>> m_meshes;
	std::vector<glm::mat4> m_bonesOffsets;
	std::unordered_map<std::string, unsigned int> m_bonesNameToIDMap;
};