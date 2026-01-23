#pragma once

#include "memory/ResourceWrapper.h"
#include "memory/AssetInfo.h"
#include "core/Core.h"
#include "core/Configurations.h"
#include "systems/SubSystem.h"
#include "fileSystem/ScopedPath.h"

#include <nlohmann/json.hpp>

class Asset;

struct EngineAPI AssetCreateDescriptor
{
public:
	virtual ~AssetCreateDescriptor() = default;

	std::string origFilePath;
	std::string assetDirectory;
	AssetType aType = AssetType::NONE;
	std::map<std::string, std::string> attributes;
	std::string name;
	bool isEngineOwned = false;
	bool isTransient = false;
	bool isCompositeAsset = false; // this asset is composed of multiple external files 
	ScopedPath targetDirectory;

	virtual nlohmann::json fillParams() const { return {}; }
	
	//timestamp
	//size
};

struct EngineAPI AssetUpdateDescriptor
{
	std::string assetDirectory;
	std::map<std::string, std::string> attributes;
	std::string name;
};

class EngineAPI Assets : public SubSystem
{
public:
	Assets();

	std::string getAlias(UUID uid) const;

	std::vector<Asset> getAllAssetsOfType(AssetType aType) const;

	std::vector<Asset> getAllAssets() const;

	void loadAssetsDatabase();

	void saveDirtyAssets();

	Asset getAssetFromPath(const std::string& path) const;

	Asset getAssetFromName(const std::string& name) const;

	Asset getAsset(UUID uuid) const;

	const AssetInfo& getInfo(UUID uuid) const;

	bool hasAsset(UUID uuid) const;

	void deleteAsset(Asset asset);

	void updateAsset(Asset asset, const AssetUpdateDescriptor& uDesc = {});

	void reimportAsset(UUID uuid);

	void makeDirty(UUID uuid);

	std::vector<Asset> getAssetDependancies(UUID uuid) const;

	//void addAssetDependency(UUID asset, UUID dependency);

	//void removeAssetDependency(UUID asset, UUID dependency);

	Asset importAsset(const std::string& fileLocation, AssetCreateDescriptor& desc);

	Asset createAsset(const ResourceWrapper<Resource>& asset, AssetCreateDescriptor& desc);

	ResourceWrapper<Resource> loadResource(const std::string& fileLocation, AssetCreateDescriptor& desc);

private:
	void addAsset(AssetInfo& aInfo);

	void updateRegistry(const AssetInfo& aInfo);

	bool importAssetInner(AssetInfo& aInfo);
private:
	std::unordered_map<UUID, AssetInfo> m_assets;
};