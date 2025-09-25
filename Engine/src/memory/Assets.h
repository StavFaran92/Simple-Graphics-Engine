#pragma once

#include "core/Engine.h"
#include "memory/ResourceWrapper.h"
#include "core/Core.h"
#include "core/Configurations.h"

#include <unordered_set>

struct AssetDescriptor
{
public:
	virtual ~AssetDescriptor() = default;

	UUID customUUID;
	std::string filePath;
	std::string assetDirectory;
	AssetType aType;
	std::map<std::string, std::string> attributes;
	std::string name;
	bool isTransient = false;
	ResourceWrapper<ResourceBase> data = ResourceWrapper<ResourceBase>::empty;
	//timestamp
	//size
};

struct AssetInfo : public AssetDescriptor
{
	bool isValid = false;
	std::string origFilePath;
	std::string fileName;
	std::string ext;
	UUID uuid;

	~AssetInfo() = default;

	AssetInfo() = default;


	AssetInfo(const AssetDescriptor& assetDesc)
		: AssetDescriptor(assetDesc)
	{
	}

private:
	friend class Assets;
	friend class ResourceBase;
	template<typename T> friend class AssetLoader;

	
};

class EngineAPI Assets
{
public:
	Assets();

	std::string getAlias(UUID uid) const;

	void importAsset(const AssetDescriptor& assetDesc);

	void addAsset(const AssetDescriptor& assetDesc);

	void updateAsset(const AssetDescriptor& assetDesc);

	std::vector<AssetInfo> getAllAssetsOfType(AssetType aType) const;

	std::vector<AssetInfo> getAllAssets() const;

	void loadAssetsDatabase();

	AssetInfo getAsset(UUID uuid) const;

	bool hasAsset(UUID uuid) const;

private:
	std::unordered_map<UUID, AssetInfo> m_assets;
};