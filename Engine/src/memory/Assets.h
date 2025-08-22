#pragma once

#include "core/Engine.h"
#include "memory/Resource.h"
#include "core/Core.h"
#include "core/Configurations.h"

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
	bool isTransient = false;
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

	std::vector<AssetInfo> getAllAssetsOfType(AssetType aType) const;

	std::vector<AssetInfo> getAllAssets() const;

	void load();

	AssetInfo getAsset(UUID uuid) const;
	bool hasAsset(UUID uuid) const;

private:
	//std::map<AssetType, std::unordered_set<AssetInfo>> m_assets;

	std::unordered_map<UUID, AssetInfo> m_assets;
};