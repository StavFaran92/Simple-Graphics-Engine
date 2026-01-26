#pragma once

#include <string>
#include "core/Core.h"
#include "memory/UUID.h"
#include "core/Configurations.h"
#include "AssetDescriptors.h"

class Asset;

template<typename T>
class AssetHandle;

struct EngineAPI AssetRecord
{
	AssetCreateDescriptor createDescriptor;
	bool isValid = false;
	std::string relativefilePath;
	std::string fullFilePath;
	std::string fileName;
	std::string ext;
	UUID uuid = EMPTY_UUID;
	nlohmann::json importSettings;
	Asset* asset = nullptr;
	ResourceID resourceID = 0;
	bool isDirty = false;

	bool m_isParsed = false;

	~AssetRecord() = default;

	AssetRecord() = default;

	AssetRecord(AssetCreateDescriptor& assetDesc);

	void parse();

	bool isParsed() const;

	void update(const AssetUpdateDescriptor& desc);
	void establishFilepath();
};

// Serialization (to JSON)
void to_json(nlohmann::json& j, const AssetRecord& asset);
void from_json(const nlohmann::json& j, AssetRecord& asset);