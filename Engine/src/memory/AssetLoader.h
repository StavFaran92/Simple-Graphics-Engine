#pragma once 

#include <string>
#include "Resource.h"
#include "Asset.h"
#include <filesystem>

// This class is responsible for all the asset work in the File I/O realm
// it enforces a strict one way policy on how to handle assets on disk,
// each asset is required to implement its own specifics using the AssetTraits proxy
template<typename T>
class AssetLoader
{
public:
	static Resource<T> import(const std::string& fileLocation, const BaseAssetParameters& params)
	{
		// Validate input
		if (fileLocation.empty() || !std::filesystem::exists(fileLocation))
		{
			logError("Invalid asset path specified.");
			return Resource<T>::empty;
		}

		// Data extract
		AssetInfo aInfo;
		if (params.customUUID.empty())
		{
			aInfo.uuid = uuid::generate_uuid_v4();
		}
		else
		{
			aInfo.uuid = params.customUUID;
		}
		AssetTraits<T>::convertAssetLoadParamsToAssetInfo(fileLocation, params, aInfo);

		// Copy + Paste
		if (!AssetTraits<T>::copyFiles(fileLocation, aInfo))
		{
			logError("Failed to copy file from {} to resource folder", fileLocation);
			return Resource<T>::empty;
		}

		// Load
		aInfo.data = AssetTraits<T>::load(aInfo);
		if (aInfo.data.isEmpty())
		{
			logError("Failed to load file {}", fileLocation);
			return Resource<T>::empty;
		}

		// Add Asset
		Engine::get()->getSubSystem<Assets>()->addAsset(aInfo);

		return aInfo.data.as<T>();
	}

	static Resource<T> loadTransient(const std::string& fileLocation, const BaseAssetParameters& settings)
	{
		// Validate input
		if (fileLocation.empty() || !std::filesystem::exists(fileLocation))
		{
			logError("Invalid asset path specified.");
			return Resource<T>::empty;
		}

		AssetInfo aInfo;
		AssetTraits<T>::convertAssetLoadParamsToAssetInfo(fileLocation, settings, aInfo);
		if (settings.customUUID.empty())
		{
			aInfo.uuid = uuid::generate_uuid_v4();
		}
		else
		{
			aInfo.uuid = settings.customUUID;
		}
		aInfo.isTransient = true;
		aInfo.filePath = fileLocation;

		return AssetTraits<T>::load(aInfo);
	}

	static void save(AssetInfo& aInfo, const Resource<T>& asset)
	{
		if (!aInfo.isTransient)
		{
			aInfo.filePath = "/" + aInfo.name + ".asset";
			AssetTraits<T>::save(aInfo, asset);
		}

		aInfo.data = asset;

		Engine::get()->getSubSystem<Assets>()->updateAsset(aInfo);

	}
};