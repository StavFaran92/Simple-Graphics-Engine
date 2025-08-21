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

	virtual void import(const std::string& fileLocation, const ImportSettings& settings) = 0;

	virtual void load(AssetInfo aInfo) = 0;
};