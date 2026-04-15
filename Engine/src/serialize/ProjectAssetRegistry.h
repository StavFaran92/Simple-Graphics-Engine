#pragma once

#include <vector>

#include <nlohmann/json.hpp>
#include "memory/UUID.h"

#include "memory/ResourceRef.h"
#include "texture/Texture.h"

struct AssetRecord;

class ProjectAssetRegistry
{
public:
	ProjectAssetRegistry(const std::string& filename);
	static std::shared_ptr<ProjectAssetRegistry> create(const std::string& filename);
	static std::shared_ptr<ProjectAssetRegistry> parse(const std::string& filename);

	std::string getFilepath() const;
	void reset();
	void load(); //????
	void save();
	//void addTexture(Resource<Texture> texture);
	void addAssetRegistry(const AssetRecord& asset);
	void updateAssetRegistry(const AssetRecord& asset);
	void removeAssetRegistry(const AssetRecord& asset);

	//void addAssociation(std::string name, UUID uuid);
	//void removeAssociation(std::string name);
	//std::unordered_map<std::string, UUID> getAssociations() const;
	std::unordered_map<std::string, UUID> getPaths() const;
	std::unordered_map<std::string, UUID> getNames() const;

	void syncPaths(const std::unordered_map<std::string, UUID>& pathsReferences);
	void syncNames(const std::unordered_map<std::string, UUID>& namesReferences);

	std::vector<AssetRecord> getAllAssetsOfType(AssetType aType) const;
	std::vector<AssetRecord> getAllAssets() const;


private:
	
	void sync();

private:
	std::string m_filename;
	nlohmann::json m_assetRegistry;
};