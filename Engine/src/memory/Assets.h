#pragma once

#include "core/Engine.h"
#include "memory/ResourceWrapper.h"
#include "core/Core.h"
#include "core/Configurations.h"

#include <unordered_set>

struct AssetInfo
{
	UUID uuid;
	UUID customUUID;
	std::string origFilePath;
	std::string filePath;
	std::string fileName;
	std::string assetDirectory;
	std::string ext;
	AssetType aType;
	bool isValid = false;
	std::map<std::string, std::string> attributes;
	std::string name;
	bool isTransient = false;
	ResourceWrapper<ResourceBase> data = ResourceWrapper<ResourceBase>::empty;
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
	ResourceWrapper<T> loadAsset(UUID uid, const std::string& path);

	AssetInfo importAsset(AssetInfo& assetInfo);

	AssetInfo addAsset(AssetInfo& assetInfo);

	AssetInfo updateAsset(AssetInfo& assetInfo);

	std::vector<AssetInfo> getAllAssetsOfType(AssetType aType) const;

	std::vector<AssetInfo> getAllAssets() const;

	void load();

	AssetInfo getAsset(UUID uuid) const;
	bool hasAsset(UUID uuid) const;

private:
	//std::map<AssetType, std::unordered_set<AssetInfo>> m_assets;

	std::unordered_map<UUID, AssetInfo> m_assets;
};