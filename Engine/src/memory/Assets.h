#pragma once

#include "memory/AssetRecord.h"
#include "core/Core.h"
#include "core/Configurations.h"
#include "systems/SubSystem.h"

class Asset;

class EngineAPI Assets : public SubSystem
{
public:
	Assets();

	std::string getAlias(UUID uid) const;

	std::vector<AssetHandle<Asset>> getAllAssetsOfType(AssetType aType) const;

	std::vector<const AssetRecord*> getAllRecordsOfType(AssetType aType) const;

	std::vector<AssetHandle<Asset>> getAllAssets() const;

	std::vector<const AssetRecord*> getAllRecords() const;

	void loadAssetsDatabase();

	void saveDirtyAssets();

	AssetHandle<Asset> getAssetFromPath(const std::string& path) const;

	AssetHandle<Asset> getAssetFromName(const std::string& name) const;

	AssetHandle<Asset> getAsset(UUID uuid) const;

	const AssetRecord& getInfo(UUID uuid) const;

	bool hasAsset(UUID uuid) const;

	void deleteAsset(AssetHandle<Asset> asset);

	void makeDirty(UUID uuid);

	void addAsset(AssetRecord& aInfo);

	void updateAsset(AssetRecord& aInfo);
private:

	void updateRegistry(const AssetRecord& aInfo);
private:
	std::unordered_map<UUID, AssetRecord> m_assets;
};