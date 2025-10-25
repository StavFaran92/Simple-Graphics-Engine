#pragma once

#include "core/Engine.h"
#include "memory/ResourceWrapper.h"
#include "memory/AssetInfo.h"
#include "core/Core.h"
#include "core/Configurations.h"
#include "systems/SubSystem.h"

#include <unordered_set>

#include <filesystem>

#include <nlohmann/json.hpp>

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

	void addAsset(AssetInfo& aInfo);

	std::vector<AssetInfo> getAllAssetsOfType(AssetType aType) const;

	std::vector<AssetInfo> getAllAssets() const;

	void loadAssetsDatabase();

	UUID getAssetFromPath(const std::string& path) const;

	UUID getAssetFromName(const std::string& name) const;

	AssetInfo getAsset(UUID uuid) const;

	bool hasAsset(UUID uuid) const;

	void deleteAsset(const AssetInfo& aInfo);

	void updateAsset(const AssetWrapper<ResourceBase>& asset, const AssetUpdateDescriptor& uDesc = {});

	AssetWrapper<ResourceBase> importAsset(const std::string& fileLocation, AssetCreateDescriptor& desc);

	AssetWrapper<ResourceBase> createAsset(const ResourceWrapper<ResourceBase>& asset, AssetCreateDescriptor& desc);

	ResourceWrapper<ResourceBase> loadResource(const std::string& fileLocation, AssetCreateDescriptor& desc);

private:
	void updateRegistry(const AssetInfo& aInfo);
private:
	std::unordered_map<UUID, AssetInfo> m_assets;
};