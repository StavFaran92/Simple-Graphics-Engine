#pragma once 

#include <string>
#include "memory/ResourceWrapper.h"
#include "Asset.h"
#include <filesystem>

// This class is responsible for all the asset work in the File I/O realm
// it enforces a strict one way policy on how to handle assets on disk,
// each asset is required to implement its own specifics using the AssetTraits proxy
template<typename T>
class AssetLoader
{
public:
	static ResourceWrapper<T> import(const std::string& fileLocation, AssetInfo& aInfo)
	{
		// Validate input
		if (fileLocation.empty() || !std::filesystem::exists(fileLocation))
		{
			logError("Invalid asset path specified.");
			return ResourceWrapper<T>::empty;
		}

		//aDesc.origFilePath = fileLocation;
		//AssetInfo aInfo(aDesc);

		std::filesystem::create_directories(Engine::get()->getProjectDirectory() + "/" + aInfo.assetDirectory);

		// Copy + Paste
		if (!AssetTraits<T>::copyFiles(fileLocation, aInfo))
		{
			logError("Failed to copy file from {} to resource folder", fileLocation);
			return ResourceWrapper<T>::empty;
		}

		// Load
		aInfo.data = AssetTraits<T>::load(aInfo);
		if (aInfo.data.isEmpty())
		{
			logError("Failed to load file {}", fileLocation);
			return ResourceWrapper<T>::empty;
		}

		// Add Asset
		Engine::get()->getSubSystem<Assets>()->addAsset(aInfo);

		return aInfo.data.as<T>();
	}

	static ResourceWrapper<T> loadTransient(const std::string& fileLocation, AssetInfo& aInfo)
	{
		// Validate input
		if (fileLocation.empty() || !std::filesystem::exists(fileLocation))
		{
			logError("Invalid asset path specified.");
			return ResourceWrapper<T>::empty;
		}

		//aDesc.origFilePath = fileLocation;
		//AssetInfo aInfo(aDesc);
		aInfo.isTransient = true;
		aInfo.filePath = fileLocation;

		ResourceWrapper<T> asset = AssetTraits<T>::load(aInfo);

		if (asset.isEmpty() || !asset.get())
		{
			logWarning("Failed to load asset from {}", fileLocation);
		}

		return asset;
	}

	static void save(AssetInfo& aInfo, const ResourceWrapper<T>& asset)
	{
		if (!aInfo.isTransient)
		{
			AssetTraits<T>::save(aInfo, asset);
		}

		aInfo.data = asset;

		Engine::get()->getSubSystem<Assets>()->updateAsset(aInfo);

	}

	static ResourceWrapper<T> create(AssetDescriptor& aDesc)
	{
		AssetInfo aInfo(aDesc);

		ResourceWrapper<T> asset = Factory<T>::createUsingCustomUUID(aInfo.uuid);

		save(aInfo, asset);

		asset.get()->m_assetInfo = aInfo;

		return asset;
	}

private:
	//static AssetInfo extractAssetInfoData(const std::string& fileLocation, const BaseAssetParameters& params)
	//{
	//	AssetInfo aInfo;
	//	aInfo.name = params.name.empty()
	//		? std::filesystem::path(fileLocation).filename().stem().string()
	//		: params.name;

	//	aInfo.origFilePath = fileLocation;

	//	aInfo.assetDirectory = params.targetDirectory;

	//	AssetTraits<T>::convertAssetLoadParamsToAssetInfo(fileLocation, params, aInfo);

	//	aInfo.filePath = (std::filesystem::path(aInfo.assetDirectory) / aInfo.fileName).generic_string();

	//	aInfo.uuid = params.customUUID.empty()
	//		? aInfo.filePath
	//		: params.customUUID;

	//	

	//	return aInfo;
	//}
};