#pragma once

#include <string>

#include "Assets.h"

class ImportSettings
{

};

class Asset
{
public:
	Asset() = default;
	virtual ~Asset() = default;

	virtual Resource<Asset> import(const std::string& fileLocation, const ImportSettings& settings) = 0;

	virtual Resource<Asset> load(AssetInfo aInfo) = 0;
};