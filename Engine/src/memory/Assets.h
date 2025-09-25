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
	mutable bool isValid = false;
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

	AssetInfo importAsset(AssetInfo& assetInfo);

	AssetInfo addAsset(const AssetInfo& assetInfo);

	AssetInfo updateAsset(const AssetInfo& assetInfo);

	std::vector<AssetInfo> getAllAssetsOfType(AssetType aType) const;

	std::vector<AssetInfo> getAllAssets() const;

	void loadAssetsDatabase();

	AssetInfo getAsset(UUID uuid) const;

	bool hasAsset(UUID uuid) const;

private:
	std::unordered_map<UUID, AssetInfo> m_assets;
};