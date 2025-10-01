#pragma once

#include "core/Engine.h"
#include "memory/ResourceWrapper.h"
#include "core/Core.h"
#include "core/Configurations.h"

#include <unordered_set>

#include <filesystem>

#include <nlohmann/json.hpp>
//#include "core/CacheSystem.h"
//#include "serialize/ProjectAssetRegistry.h"

using json = nlohmann::json;
using namespace nlohmann::literals;

struct AssetInfo;

struct EngineAPI AssetDescriptor
{
public:
	virtual ~AssetDescriptor() = default;

	UUID customUUID;
	std::string filePathHint;
	std::string origFilePath;
	std::string assetDirectory;
	AssetType aType;
	std::map<std::string, std::string> attributes;
	std::string name;
	bool isTransient = false;
	mutable ResourceWrapper<ResourceBase> data = ResourceWrapper<ResourceBase>::empty;

	virtual json fillParams() const { return {}; }

	AssetInfo parse();
	
	//timestamp
	//size
};

struct EngineAPI AssetUpdateDescriptor
{
	std::string assetDirectory;
	std::map<std::string, std::string> attributes;
	std::string name;
};

struct EngineAPI AssetInfo : public AssetDescriptor
{
	bool isValid = false;
	std::string filePath;
	std::string fileName;
	std::string ext;
	UUID uuid;
	nlohmann::json importSettings;

	~AssetInfo() = default;

	AssetInfo() = default;

	AssetInfo(const AssetDescriptor& assetDesc);

	void update(const AssetUpdateDescriptor& uDesc);

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

	void addAsset(AssetInfo& aInfo);

	std::vector<AssetInfo> getAllAssetsOfType(AssetType aType) const;

	std::vector<AssetInfo> getAllAssets() const;

	void loadAssetsDatabase();

	UUID getAssetFromPath(const std::string& path) const;

	AssetInfo getAsset(UUID uuid) const;

	bool hasAsset(UUID uuid) const;

	void updateAsset(const ResourceWrapper<ResourceBase>& asset, const AssetUpdateDescriptor& uDesc = {});

	ResourceWrapper<ResourceBase> importAsset(const std::string& fileLocation, AssetInfo& aInfo);

	ResourceWrapper<ResourceBase> createAsset(const ResourceWrapper<ResourceBase>& asset, AssetInfo& aInfo);

private:
	void updateRegistry(const AssetInfo& aInfo);
private:
	std::unordered_map<UUID, AssetInfo> m_assets;
};