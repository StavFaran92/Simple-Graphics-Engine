#pragma once

#include <vector>

#include "core/Core.h"
#include "geometry/Mesh.h"
#include "memory/Resource.h"
#include "memory/Asset.h"

class EngineAPI MeshCollection : public Asset
{
public:
	void addMesh(const std::shared_ptr<Mesh>& mesh);

	std::shared_ptr<Mesh> getPrimaryMesh() const;

	std::vector<std::shared_ptr<Mesh>> getMeshes() const;

	size_t getNumOfVertices() const;

	void addBonesInfo(const std::vector<glm::mat4>& bonesOffsets, const std::unordered_map<std::string, unsigned int>& bonesNameToIDMap);

	std::vector<glm::mat4> getBoneOffsets() const;

	int getBoneID(const std::string& boneName) const;

	// Inherited via Asset
	Resource<Asset> import(const std::string& fileLocation, const ImportSettings& settings) override;
	Resource<Asset> load(AssetInfo aInfo) override;
	

private:
	std::vector<std::shared_ptr<Mesh>> m_meshes;
	std::vector<glm::mat4> m_bonesOffsets;
	std::unordered_map<std::string, unsigned int> m_bonesNameToIDMap;
};