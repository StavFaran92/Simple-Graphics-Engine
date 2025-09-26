#pragma once

#include "core/Engine.h"
#include "memory/ResourceWrapper.h"
#include "core/Core.h"
#include "core/Configurations.h"

#include <unordered_set>

#include <filesystem>

#include <nlohmann/json.hpp>

using json = nlohmann::json;
using namespace nlohmann::literals;

struct AssetDescriptor
{
public:
	virtual ~AssetDescriptor() = default;

	UUID customUUID;
	std::string filePathHint;
	std::string origFilePath;
	std::string assetDirectory;
	AssetType aType;
	std::map<std::string, std::string> attributes;
	std::string name;
	bool isTransient = false;
	mutable ResourceWrapper<ResourceBase> data = ResourceWrapper<ResourceBase>::empty;

	virtual json fillParams() const { return {}; }
	
	//timestamp
	//size
};

struct AssetInfo : public AssetDescriptor
{
	bool isValid = false;
	std::string filePath;
	std::string fileName;
	std::string ext;
	UUID uuid;
	nlohmann::json importSettings;

	~AssetInfo() = default;

	AssetInfo() = default;


	AssetInfo(const AssetDescriptor& assetDesc)
		: AssetDescriptor(assetDesc)
	{
		importSettings = assetDesc.fillParams();

		if (aType == AssetType::NONE)
		{
			logError("Asset type cannot be NONE.");
			return;
		}

		if (!origFilePath.empty())
		{
			auto& path = std::filesystem::path(origFilePath);

			// Extract Name
			if (name.empty())
			{
				name = path.filename().stem().string();
			}
			ext = path.extension().string();
			fileName = path.filename().string();
		}

		if (name.empty())
		{
			name = uuid::generate_uuid_v4();
		}

		if (ext.empty())
		{
			if (!filePathHint.empty())
			{
				ext = std::filesystem::path(filePathHint).extension().string();
			}

			ext = getExtensionFromType(aType);

			if (ext.empty())
			{
				logError("Asset extension cannot be empty.");
				return;
			}
		}

		

		fileName = name + ext;

		if (!assetDirectory.empty())
		{
			filePath += assetDirectory + "/";
		}
		filePath += fileName;

		if (!customUUID.empty())
		{
			uuid = customUUID;
		}
		else
		{
			uuid = filePath;
		}
	}

private:
	friend class Assets;
	friend class ResourceBase;
	template<typename T> friend class AssetLoader;
};

class EngineAPI Assets
{
public:
	Assets();

	std::string getAlias(UUID uid) const;

	void importAsset(const AssetDescriptor& assetDesc);

	void addAsset(const AssetDescriptor& assetDesc);

	void updateAsset(const AssetDescriptor& assetDesc);

	std::vector<AssetInfo> getAllAssetsOfType(AssetType aType) const;

	std::vector<AssetInfo> getAllAssets() const;

	void loadAssetsDatabase();

	AssetInfo getAsset(UUID uuid) const;

	bool hasAsset(UUID uuid) const;

private:
	std::unordered_map<UUID, AssetInfo> m_assets;
};