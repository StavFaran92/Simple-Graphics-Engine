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
	std::string name;
	UUID uuid = EMPTY_UUID;
	std::string sourcePath;
	AssetType aType = AssetType::NONE;
	std::map<std::string, std::string> engineAttributes;
	bool isEngineOwned = false;
	bool isTransient = false;
	bool isCompositeAsset = false; // this asset is composed of multiple external files 
	ScopedPath targetDirectory;
	std::string assetDirectory; // todo consider remove
	std::string relativefilePath;
	//std::string fullFilePath;
	std::string fileName;
	std::string ext;
	nlohmann::json importSettings;
	Asset* asset = nullptr;
	ResourceID resourceID = 0;


	~AssetRecord() = default;

	AssetRecord() = default;

	AssetRecord(AssetCreateDescriptor& assetDesc);

	void parse();
	bool isParsed() const;

	void makeDirty();
	bool isDirty() const;

	void update(const AssetUpdateDescriptor& desc);
	//void establishFilepath();

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(AssetRecord,
		name,
		uuid,
		sourcePath,
		aType,
		engineAttributes,
		isEngineOwned,
		assetDirectory,
		fileName,
		relativefilePath,
		ext
	);
private:
	friend class Assets;

	bool m_isDirty = false;
	bool m_isParsed = false;
};

// Serialization (to JSON)
//void to_json(nlohmann::json& j, const AssetRecord& asset);
//void from_json(const nlohmann::json& j, AssetRecord& asset);