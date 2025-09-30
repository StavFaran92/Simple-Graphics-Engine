#pragma once

#include "core/Engine.h"
#include "memory/ResourceWrapper.h"
#include "core/Core.h"
#include "core/Configurations.h"

#include <unordered_set>

#include <filesystem>

#include <nlohmann/json.hpp>
//#include "core/CacheSystem.h"
//#include "serialize/ProjectAssetRegistry.h"

using json = nlohmann::json;
using namespace nlohmann::literals;

struct AssetInfo;

struct EngineAPI AssetDescriptor
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

	AssetInfo parse();
	
	//timestamp
	//size
};

struct EngineAPI AssetUpdateDescriptor
{
	std::string assetDirectory;
	std::map<std::string, std::string> attributes;
	std::string name;
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

		if (!customUUID.empty())
		{
			uuid = customUUID;
		}
		else
		{
			uuid = uuid::generate_uuid_v4();
		}

		if (name.empty())
		{
			name = uuid;
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

		filePath = "";
		if (!assetDirectory.empty())
		{
			filePath += assetDirectory + "/";
		}
		filePath += fileName;
	}

	void update(const AssetUpdateDescriptor& uDesc);

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

	//void importAsset(AssetInfo& aInfo);

	void addAsset(AssetInfo& aInfo);

	//void updateAsset(const ResourceWrapper<ResourceBase>& asset, const AssetUpdateDescriptor& uDesc);

	std::vector<AssetInfo> getAllAssetsOfType(AssetType aType) const;

	std::vector<AssetInfo> getAllAssets() const;

	void loadAssetsDatabase();

	UUID getAssetFromPath(const std::string& path) const;

	AssetInfo getAsset(UUID uuid) const;

	bool hasAsset(UUID uuid) const;

	template<typename T>
	void updateAsset(const ResourceWrapper<T>& asset, const AssetUpdateDescriptor& uDesc)
	{
		AssetInfo aInfo = getAsset(asset.getUID());
		aInfo.update(uDesc);

		if (!aInfo.isTransient)
		{
			AssetTraits<T>::save(asset, aInfo);
			updateRegistry(aInfo);
		}

		m_assets[aInfo.uuid] = aInfo;

		logInfo("Successfully Updated asset: '" + aInfo.name + "'.");
	}

	template<typename T>
	ResourceWrapper<T> importAsset(const std::string& fileLocation, AssetInfo& aInfo)
	{
		// Validate input
		if (fileLocation.empty() || !std::filesystem::exists(fileLocation))
		{
			logError("Invalid asset path specified.");
			return ResourceWrapper<T>::empty;
		}

		if (!aInfo.isTransient)
		{
			std::filesystem::create_directories(Engine::get()->getProjectDirectory() + "/" + aInfo.assetDirectory);

			// Copy + Paste
			if (!AssetTraits<T>::copyFiles(fileLocation, aInfo))
			{
				logError("Failed to copy file from {} to resource folder", fileLocation);
				return ResourceWrapper<T>::empty;
			}
		}
		else
		{
			aInfo.filePath = fileLocation;
		}

		// Load
		ResourceWrapper<T> asset = AssetTraits<T>::load(aInfo);
		if (asset.isEmpty() || !asset.get())
		{
			logError("Failed to load file {}", fileLocation);
			return ResourceWrapper<T>::empty;
		}

		aInfo.data = asset;

		// Add Asset
		addAsset(aInfo);

		return asset;
	}

	template<typename T>
	ResourceWrapper<T> createAsset(const ResourceWrapper<T>& asset, AssetInfo& aInfo)
	{
		if (!aInfo.isTransient)
		{
			AssetTraits<T>::save(asset, aInfo);
		}

		asset.get()->m_assetInfo = aInfo;
		addAsset(aInfo);

		return asset;
	}

private:
	void updateRegistry(const AssetInfo& aInfo);
private:
	std::unordered_map<UUID, AssetInfo> m_assets;
};