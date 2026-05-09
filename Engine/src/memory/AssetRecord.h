#pragma once

#include <string>
#include "core/Core.h"
#include "memory/UUID.h"
#include "core/Configurations.h"
#include "AssetDescriptors.h"
#include "memory/Ref.h"

class Asset;

template<typename T>
class AssetRef;

struct EngineAPI AssetRecord
{
	std::string name;
	UUID uuid = EMPTY_UUID;
	std::string sourcePath;
	AssetType aType = AssetType::NONE;
	bool isEngineOwned = false;
	bool isTransient = false;
	std::string assetDirectory;
	std::string ext;
	AssetVisibility visibility = AssetVisibility::Public; //only relevant for engine assets
	Ref<Asset> asset;
	ResourceID resourceID = 0;


	~AssetRecord() = default;

	AssetRecord() = default;

	AssetRecord(const AssetBuildDescriptor& assetDesc);

	void parse();
	bool isParsed() const;

	// Makes the asset both resource dirty and serialization dirty.
	void makeDirty();

	// Resource now matches the asset's changes and is therefore not dirty anymore.
	void sync();

	// This means the Asset has changed but the file data on disk does not contains the asset changes.
	bool isSerializationDirty() const;

	// This means the Asset has changed and therefore the resource is not updated with the changes.
	bool isResourceDirty() const;

	bool isVisible() const;

	std::string getAbsolutePath() const;
	ScopedPath getScopedPath() const;
	std::string getFilename() const;
		

private:
	friend class Assets;

	bool m_isSerializationDirty = false;
	bool m_isResourceDirty = true;

	bool m_isParsed = false;
};

// Serialization (to JSON)
void to_json(nlohmann::json& j, const AssetRecord& asset);
void from_json(const nlohmann::json& j, AssetRecord& asset);