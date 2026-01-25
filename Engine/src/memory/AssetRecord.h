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
class Asset;


template<typename T>
class AssetHandle;

using json = nlohmann::json;

struct EngineAPI AssetRecord
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
	std::shared_ptr<Asset> asset;

	bool isDirty = false;

	~AssetRecord() = default;

	AssetRecord() = default;

	AssetRecord(const AssetCreateDescriptor& assetDesc);

	const Asset* getAsset() const;
	void update(const AssetUpdateDescriptor& desc);
	void establishFilepath();
};

// Serialization (to JSON)
void to_json(nlohmann::json& j, const AssetRecord& asset);
void from_json(const nlohmann::json& j, AssetRecord& asset);