#include "Asset.h"

#include "core/Logger.h"
#include "memory/Assets.h"
#include "core/Engine.h"

Asset::Asset(AssetCreateDescriptor* desc)
{
	*m_createDesc = *desc;
}

bool Asset::importAssetInner(AssetRecord& aInfo)
{
	std::string fileLocation = aInfo.createDescriptor->origFilePath;

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
	aInfo.createDescriptor->origFilePath = fileLocation;
	aInfo.asset = this;
	aInfo.parse();

	if (!importAssetInner(aInfo))
	{
		return AssetHandle<Asset>::empty;
	}

	AssetHandle<Asset> asset(aInfo.uuid);

	return asset;
}
