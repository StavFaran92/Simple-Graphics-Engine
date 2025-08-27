#pragma once 

#include <string>
#include "Resource.h"
#include "Asset.h"

class AssetLoader
{
public:
	virtual void convertAssetLoadParamsToAssetInfo(const std::string& fileLocation, const BaseAssetParameters& params, AssetInfo& aInfo) = 0;

	virtual std::string copyFileToResourceFolder(const std::string& fileLocation, AssetInfo& aInfo);

	virtual Resource<Asset> import(const std::string& fileLocation, const BaseAssetParameters& params);

	virtual Resource<Asset> load(AssetInfo& aInfo) = 0;
};