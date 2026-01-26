#pragma once

#include <string>

#include "memory/AssetHandle.h"
#include "AssetFactory.h"

class Asset
{
public:
	using ResourceType = Resource;

	Asset(const AssetCreateDescriptor& desc);

	AssetHandle<Asset> importAsset(const std::string& fileLocation);

	AssetHandle<Asset> createAsset(const ResourceWrapper<Resource>& asset);

	void updateAsset(const AssetHandle<Asset>& asset, const AssetUpdateDescriptor& uDesc);

	void reimportAsset();

	virtual void save(const AssetRecord& aInfo) = 0;

	const AssetCreateDescriptor& getDescriptor() const;

	UUID getUUID() const;
protected:
	virtual bool copyFiles(const std::string& fileLocation, AssetRecord& aInfo) = 0;

	virtual std::string getRecommendedExtension(const AssetRecord& aInfo) { return getExtensionFromType(aInfo.createDescriptor.aType); };

	bool importAssetInner(AssetRecord& aInfo);

protected:
	AssetCreateDescriptor m_createDesc;

	UUID m_uuid = EMPTY_UUID;
};