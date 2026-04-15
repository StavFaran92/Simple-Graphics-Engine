#pragma once 

#include <string>
#include "memory/ResourceRef.h"
#include "Asset.h"
#include <filesystem>

// This class is responsible for all the asset work in the File I/O realm
// it enforces a strict one way policy on how to handle assets on disk,
// each asset is required to implement its own specifics using the AssetTraits proxy
template<typename T>
class AssetLoader
{
public:
	//static ResourceWrapper<T> import(const std::string& fileLocation, AssetInfo& aInfo)
	//{
	//	// Validate input
	//	if (fileLocation.empty() || !std::filesystem::exists(fileLocation))
	//	{
	//		logError("Invalid asset path specified.");
	//		return ResourceWrapper<T>::empty;
	//	}

	//	if (!aInfo.isTransient)
	//	{
	//		std::filesystem::create_directories(Engine::get()->getProjectDirectory() + "/" + aInfo.assetDirectory);

	//		// Copy + Paste
	//		if (!AssetTraits<T>::copyFiles(fileLocation, aInfo))
	//		{
	//			logError("Failed to copy file from {} to resource folder", fileLocation);
	//			return ResourceWrapper<T>::empty;
	//		}
	//	}
	//	else
	//	{
	//		aInfo.filePath = fileLocation;
	//	}

	//	// Load
	//	ResourceWrapper<T> asset = AssetTraits<T>::load(aInfo);
	//	if (asset.isEmpty() || !asset.get())
	//	{
	//		logError("Failed to load file {}", fileLocation);
	//		return ResourceWrapper<T>::empty;
	//	}

	//	aInfo.data = asset;

	//	// Add Asset
	//	Engine::get()->getSubSystem<Assets>()->addAsset(aInfo);

	//	return asset;
	//}

	//static void updateAsset(const ResourceWrapper<T>& asset, const AssetUpdateDescriptor& uDesc)
	//{
	//	AssetInfo aInfo = Engine::get()->getSubSystem<Assets>()->getAsset(asset.getUID());
	//	aInfo.update(uDesc);

	//	if (!aInfo.isTransient)
	//	{
	//		AssetTraits<T>::save(asset, aInfo);
	//	}

	//	Engine::get()->getSubSystem<Assets>()->updateAsset(aInfo);
	//}

	//static ResourceWrapper<T> createAsset(AssetInfo& aInfo)
	//{
	//	ResourceWrapper<T> asset = Factory<T>::createUsingCustomUUID(aInfo.uuid);

	//	if (!aInfo.isTransient)
	//	{
	//		AssetTraits<T>::save(asset, aInfo);
	//	}

	//	asset.get()->m_assetInfo = aInfo;
	//	Engine::get()->getSubSystem<Assets>()->addAsset(aInfo);

	//	return asset;
	//}
};