#include "AssetLoader.h"
#include "Asset.h"

//template<typename T>
//Resource<T> AssetLoader<T>::import(const std::string& fileLocation, const BaseAssetParameters& params)
//{
//	// Validate input
//	if (fileLocation.empty())
//	{
//		logError("Invalid texture name, cannot be empty.");
//		return Resource<T>::empty;
//	}
//
//	// Data extract
//	AssetInfo aInfo;
//	aInfo.uuid = uuid::generate_uuid_v4();
//	AssetTraits<T>::convertAssetLoadParamsToAssetInfo(fileLocation, params, aInfo);
//
//	// Copy + Paste
//	if (!AssetTraits<T>::copyFiles(fileLocation, aInfo))
//	{
//		logError("Failed to copy file from {} to resource folder", fileLocation);
//		return Resource<T>::empty;
//	}
//
//	// Load
//	aInfo.data = AssetTraits<T>::load(aInfo);
//	if (aInfo.data.isEmpty())
//	{
//		logError("Failed to load file {}", fileLocation);
//		return Resource<T>::empty;
//	}
//
//	// Add Asset
//	Engine::get()->getSubSystem<Assets>()->addAsset(aInfo);
//
//	return aInfo.data.as<T>();
//}
//
//template<typename T>
//Resource<T> AssetLoader<T>::loadTransient(const std::string& fileLocation, const BaseAssetParameters& settings)
//{
//	AssetInfo aInfo;
//	AssetTraits<T>::convertAssetLoadParamsToAssetInfo(fileLocation, settings, aInfo);
//	aInfo.uuid = uuid::generate_uuid_v4();
//	aInfo.isTransient = true;
//	aInfo.filePath = fileLocation;
//
//	return AssetTraits<T>::load(aInfo);
//}
//
//template<typename T>
//Resource<T> AssetLoader<T>::load(AssetInfo& aInfo)
//{
//	return AssetTraits<T>::load(aInfo);
//}