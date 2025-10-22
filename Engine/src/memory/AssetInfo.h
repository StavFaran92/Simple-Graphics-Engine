#pragma once

#include <string>
#include "core/Core.h"
#include "memory/UUID.h"
#include "core/Configurations.h"
#include "memory/ResourceBase.h"

struct AssetCreateDescriptor;
struct AssetUpdateDescriptor;


template<typename T>
class AssetWrapper;

struct EngineAPI AssetInfo
{
	bool isValid = false;
	std::string relativefilePath;
	std::string fullFilePath;
	std::string fileName;
	std::string ext;
	UUID uuid = EMPTY_UUID;
	std::string origFilePath;
	std::string assetDirectory;
	AssetType aType = AssetType::NONE;
	std::map<std::string, std::string> attributes;
	std::string name;
	bool isEngineOwned = false;
	bool isTransient = false;
	nlohmann::json importSettings;
	//AssetWrapper<ResourceBase> data = AssetWrapper<ResourceBase>::empty;

	~AssetInfo() = default;

	AssetInfo() = default;

	AssetInfo(const AssetCreateDescriptor& assetDesc);

	AssetWrapper<ResourceBase> data() const;
	void update(const AssetUpdateDescriptor& desc);
	void establishFilepath();
};