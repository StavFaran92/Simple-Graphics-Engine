#include "Asset.h"

#include "core/Logger.h"
#include "memory/Assets.h"
#include "core/Engine.h"

Asset::Asset(const AssetCreateDescriptor& desc)
	:m_createDesc (desc)
{
}

bool Asset::importAssetInner(AssetRecord& aInfo)
{
	std::string fileLocation = aInfo.createDescriptor.origFilePath;

	// Validate input
	if (fileLocation.empty() || !std::filesystem::exists(fileLocation))
	{
		logError("Invalid asset path specified.");
		return false;
	}

	// Copy + Paste
	if (!copyFiles(fileLocation, aInfo))
	{
		logError("Failed to copy file from {} to resource folder", fileLocation);
		return false;
	}

	Engine::get()->getSubSystem<Assets>()->addAsset(aInfo);

	return true;
}

AssetHandle<Asset> Asset::importAsset(const std::string& fileLocation)
{
	AssetRecord aInfo(m_createDesc);
	aInfo.createDescriptor.origFilePath = fileLocation;
	aInfo.asset = this;
	aInfo.parse();

	if (!importAssetInner(aInfo))
	{
		return AssetHandle<Asset>::empty;
	}

	// Store for later use
	m_uuid = aInfo.uuid;

	AssetHandle<Asset> asset(aInfo.uuid);

	return asset;
}

void Asset::reimportAsset()
{
	AssetRecord aInfo = Engine::get()->getSubSystem<Assets>()->getAsset(m_uuid).info();
	importAssetInner(aInfo);
}

const AssetCreateDescriptor& Asset::getDescriptor() const
{
	return m_createDesc;
}

UUID Asset::getUUID() const
{
	return m_uuid;
}

AssetHandle<Asset> Asset::createAsset(const ResourceWrapper<Resource>& resource)
{
	// Add asset info
	AssetRecord aInfo(m_createDesc);
	aInfo.asset = this;
	aInfo.parse();
	aInfo.resourceID = resource.getUID();

	// Store for later use
	m_uuid = aInfo.uuid;

	// Save asset
	save(aInfo);

	AssetHandle<Asset> asset(aInfo.uuid);
	return asset;
}


void Asset::updateAsset(const AssetHandle<Asset>& asset, const AssetUpdateDescriptor& uDesc)
{
	AssetRecord aInfo = Engine::get()->getSubSystem<Assets>()->getAsset(m_uuid).info();
	aInfo.update(uDesc);

	// Save asset
	save(aInfo);

	Engine::get()->getSubSystem<Assets>()->updateAsset(aInfo);
}