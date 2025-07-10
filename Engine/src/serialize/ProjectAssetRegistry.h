#pragma once

#include <vector>

#include <nlohmann/json.hpp>
#include "UUID.h"

#include "Resource.h"
#include "Texture.h"

struct AssetInfo;

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
	void addAssetRegistry(AssetInfo asset);
	void addAssociation(std::string name, UUID uuid);
	std::vector<AssetInfo> getAllAssetsOfType(AssetType aType) const;
	//std::vector<UUID> getMeshList() const;
	//std::vector<UUID> getTextureList() const;
	//std::vector<UUID> getAnimationList() const;
	std::unordered_map<std::string, UUID> getAssociations() const;


private:
	
	void sync();

private:
	std::string m_filename;
	nlohmann::json m_assetRegistry;
};