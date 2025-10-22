#pragma once

#include <string>

#include "Assets.h"
#include "AssetFactory.h"
#include "memory/AssetWrapper.h"

#include <nlohmann/json.hpp>

using json = nlohmann::json;
using namespace nlohmann::literals;

//struct BaseAssetParameters
//{
//	std::string name;
//	UUID customUUID;
//	std::string targetDirectory;
//	bool isTransient = false;
//
//	void fillAssetInfo(AssetInfo& aInfo) const
//	{
//		if (!name.empty())
//		{
//			aInfo.name = name;
//		}
//
//		aInfo.assetDirectory = targetDirectory;
//
//		aInfo.filePath = (std::filesystem::path(aInfo.assetDirectory) / aInfo.fileName).generic_string();
//
//		aInfo.importSettings = fillParams();
//	}
//	virtual json fillParams() const { return {}; };
//
//	virtual ~BaseAssetParameters() = default;
//};

template<AssetType T>
class AssetFnRegister
{
public:
	AssetFnRegister(const AssetFactory::LoadFn& fn)
	{
		AssetFactory::registerLoadFunc(T, fn);
	}

	static AssetFnRegister<T> staticRegister;
};

template<AssetType T>
AssetFnRegister<T> AssetFnRegister<T>::staticRegister;

class AssetManager {
public:
	virtual bool copyFiles(const std::string& fileLocation, AssetInfo&) = 0;
	virtual ResourceWrapper<ResourceBase> load(AssetInfo& aInfo) = 0;
	virtual void save(const AssetWrapper<ResourceBase>& asset, const AssetInfo& aInfo) {};
};