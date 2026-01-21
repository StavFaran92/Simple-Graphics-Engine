#pragma once

#include <string>
#include "core/Core.h"
#include "memory/UUID.h"
#include "core/Configurations.h"
#include "memory/ResourceBase.h"
#include "memory/ResourceWrapper.h"
#include "fileSystem/ScopedPath.h"

struct AssetCreateDescriptor;
struct AssetUpdateDescriptor;


template<typename T>
class AssetWrapper;

using json = nlohmann::json;

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
	ScopedPath targetDirectory;
	AssetType aType = AssetType::NONE;
	std::map<std::string, std::string> attributes;
	std::string name;
	bool isEngineOwned = false;
	bool isTransient = false;
	bool isCompositeAsset = false; // this asset is composed of multiple external files 
	nlohmann::json importSettings;
	std::vector<UUID> assetsDependancies;
	ResourceWrapper<ResourceBase> resource = ResourceWrapper<ResourceBase>::empty;

	bool isDirty = false;

	~AssetInfo() = default;

	AssetInfo() = default;

	AssetInfo(const AssetCreateDescriptor& assetDesc);

	AssetWrapper<ResourceBase> data() const;
	void update(const AssetUpdateDescriptor& desc);
	void establishFilepath();
};

// Serialization (to JSON)
void to_json(nlohmann::json& j, const AssetInfo& asset);
void from_json(const nlohmann::json& j, AssetInfo& asset);