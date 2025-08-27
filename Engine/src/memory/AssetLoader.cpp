#include "AssetLoader.h"

bool AssetLoader::copyFileToResourceFolder(const std::string& fileLocation, AssetInfo&)
{
	throw std::runtime_error("Not implemented yet");
}

Resource<Asset> AssetLoader::import(const std::string& fileLocation, const BaseAssetParameters& params)
{
	// Validate input
	if (fileLocation.empty())
	{
		logError("Invalid texture name, cannot be empty.");
		return Resource<Asset>::empty;
	}

	// Data extract
	AssetInfo aInfo;
	aInfo.uuid = uuid::generate_uuid_v4();
	convertAssetLoadParamsToAssetInfo(fileLocation, params, aInfo);

	// Copy + Paste
	if (!copyFileToResourceFolder(fileLocation, aInfo))
	{
		logError("Failed to copy file from {} to resource folder", fileLocation);
		return Resource<Asset>::empty;
	}

	// Load
	aInfo.data = load(aInfo);
	if (aInfo.data.isEmpty())
	{
		logError("Failed to load file {}", fileLocation);
		return Resource<Asset>::empty;
	}

	// Add Asset
	Engine::get()->getSubSystem<Assets>()->addAsset(aInfo);

	return aInfo.data;
}

Resource<Asset> AssetLoader::loadTransient(const std::string& fileLocation, const BaseAssetParameters& settings)
{
	AssetInfo aInfo;
	convertAssetLoadParamsToAssetInfo(fileLocation, settings, aInfo);
	aInfo.uuid = uuid::generate_uuid_v4();
	aInfo.isTransient = true;
	aInfo.filePath = fileLocation;

	return load(aInfo);
}
