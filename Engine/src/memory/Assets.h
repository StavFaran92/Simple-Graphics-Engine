#pragma once

#include "memory/AssetRecord.h"
#include "core/Core.h"
#include "core/Configurations.h"
#include "systems/SubSystem.h"
#include "memory/ResourcePipeline.h"

class Asset;

class EngineAPI Assets : public SubSystem
{
public:
	Assets();

	AssetHandle<Asset> createAsset(AssetBuildDescriptor& desc, ResourceBuildDescriptor& resourceDesc);

	AssetHandle<Asset> importAsset(AssetBuildDescriptor& desc, ResourceLoadDescriptor& resourceDesc);

	void updateAsset(UUID uuid, AssetUpdateDescriptor& desc, ResourceBuildDescriptor* resourceDesc = nullptr);

	void deleteAsset(UUID uuid);

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

	void makeDirty(UUID uuid);

	void sync(UUID uuid);

	void bindResourceToAsset(UUID uuid, ResourceID resID);

private:

	AssetHandle<Asset> createAssetsFromImportNode(const ImportNode& node, const AssetBuildDescriptor& rootDesc);

	AssetHandle<Asset> createAssetAndChildrenFromNodeRecursive(const ImportNode& node, const AssetBuildDescriptor& rootDesc);

	void updateRegistry(const AssetRecord& aInfo);

	void addAsset(AssetRecord& aInfo);

	void updateAssetInner(AssetRecord& aInfo);

private:
	std::unordered_map<UUID, AssetRecord> m_assets;
};