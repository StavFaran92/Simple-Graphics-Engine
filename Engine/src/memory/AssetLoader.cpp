#include "AssetLoader.h"

std::string AssetLoader::copyFileToResourceFolder(const std::string& fileLocation, AssetInfo&)
{
	throw std::runtime_error("Not implemented yet");
}

Resource<Asset> AssetLoader::import(const std::string& fileLocation, const BaseAssetParameters& params)
{
	// Validate input
	if (fileLocation.empty())
	{
		logWarning("Invalid texture name, cannot be empty.");
		return Resource<Asset>::empty;
	}

	// Data extract
	AssetInfo aInfo;
	aInfo.uuid = uuid::generate_uuid_v4();
	convertAssetLoadParamsToAssetInfo(fileLocation, params, aInfo);

	// Copy + Paste
	copyFileToResourceFolder(fileLocation, aInfo);

	// Load
	aInfo.data = load(aInfo);

	// Add Asset
	Engine::get()->getSubSystem<Assets>()->addAsset(aInfo);

	// Validate output
	if (!aInfo.isValid)
	{
		return Resource<Asset>::empty;
	}

	return aInfo.data;
}
