#pragma once

#include "Engine.h"
#include "Resource.h"
#include "Core.h"
#include "Configurations.h"

#include <unordered_set>

struct AssetInfo
{
	UUID uuid;
	std::string origFilePath;
	std::string filePath;
	AssetType aType;
	bool isValid = false;
	std::map<std::string, std::string> attributes;
	std::string name;
	//bool isClient = false
	//timestamp
	//size
	//etc..
};

class EngineAPI Assets
{
public:
	Assets();

	std::string getAlias(UUID uid) const;

	template<typename T>
	Resource<T> loadAsset(UUID uid, const std::string& path);

	AssetInfo importAsset(AssetInfo assetInfo);

	AssetInfo addAsset(AssetInfo assetInfo);

	std::vector<std::string> getAllAssetsOfType(AssetType aType) const;

private:
	std::map<AssetType, std::unordered_set<UUID>> m_assets;
};