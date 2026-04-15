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
	Ref<Asset> asset;
	ResourceID resourceID = 0;


	~AssetRecord() = default;

	AssetRecord() = default;

	AssetRecord(const AssetBuildDescriptor& assetDesc);

	void parse();
	bool isParsed() const;

	void makeDirty();
	void sync();
	bool isSerializationDirty() const;
	bool isResourceDirty() const;
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