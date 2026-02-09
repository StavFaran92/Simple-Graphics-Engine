#pragma once

#include <string>
#include "memory/AssetRecord.h"

class AssetManager {
public:
	virtual bool copyFiles(const std::string& fileLocation, const AssetRecord& record) = 0;
	virtual Asset* createAsset(const AssetCreateDescriptor& desc) = 0;
	virtual bool saveToFile(ResourceWrapper<Resource>, AssetRecord& record) = 0;
};