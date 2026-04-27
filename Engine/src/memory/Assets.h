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

	AssetRef<Asset> createAsset(AssetBuildDescriptor& desc, ResourceBuildDescriptor& resourceDesc);

	AssetRef<Asset> importAsset(AssetBuildDescriptor& desc, ResourceLoadDescriptor& resourceDesc);

	void updateAsset(UUID uuid, AssetUpdateDescriptor& desc, ResourceBuildDescriptor* resourceDesc = nullptr);

	void renameAsset(UUID uuid, const std::string& newName);

	void reimportAsset(UUID uuid);

	void moveAsset(UUID uuid, const ScopedPath& newDirectory);

	void deleteAsset(UUID uuid);

	std::string getAlias(UUID uid) const;

	std::vector<AssetRef<Asset>> getAllAssetsOfType(AssetType aType) const;

	std::vector<const AssetRecord*> getAllRecordsOfType(AssetType aType) const;

	std::vector<AssetRef<Asset>> getAllAssets() const;

	std::vector<const AssetRecord*> getAllRecords() const;

	void loadAssetsDatabase();

	void saveDirtyAssets();

	void syncAllAssets();

	AssetRef<Asset> getAssetFromPath(const std::string& path) const;

	AssetRef<Asset> getAssetFromName(const std::string& name) const;

	AssetRef<Asset> getAsset(UUID uuid) const;

	const AssetRecord& getInfo(UUID uuid) const;

	bool hasAsset(UUID uuid) const;

	void makeDirty(UUID uuid);

	void sync(UUID uuid);

	void bindResourceToAsset(UUID uuid, ResourceID resID);

	AssetRef<Asset> bakeAssetFromResource(const ResourceRef<Resource>& resource, const std::string& name = "", const ScopedPath& targetDirectory = ScopedPath::EnginePath());

private:

	AssetRef<Asset> createAssetsFromImportNode(const ImportNode& node, const AssetBuildDescriptor& rootDesc);

	AssetRef<Asset> createAssetAndChildrenFromNodeRecursive(const ImportNode& node, const AssetBuildDescriptor& rootDesc);

	void updateRegistry(const AssetRecord& aInfo);

	void addAsset(AssetRecord& aInfo);

	void updateAssetInner(AssetRecord& aInfo);

private:
	std::unordered_map<UUID, AssetRecord> m_assets;
};