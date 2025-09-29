#pragma once

#include <string>

#include "Assets.h"
#include "AssetFactory.h"

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

template<typename T>
struct AssetTraits {
	static bool copyFiles(const std::string& fileLocation, AssetInfo&);
	static ResourceWrapper<T> load(AssetInfo& aInfo);
	static void save(const ResourceWrapper<T>& asset, const std::string& path);
};

class ResourceBase
{
public:
	ResourceBase() = default;
	virtual ~ResourceBase() = default;

	static ResourceWrapper<ResourceBase> import(const std::string& fileLocation, AssetInfo& params);

	//virtual Resource<Asset> load(AssetInfo aInfo) = 0;

	// CRTP
	template <typename T>
	static ResourceWrapper<T> import(const std::string& fileLocation, AssetInfo& params) {
		return T::import(fileLocation, params);
	}

	template <typename T>
	static ResourceWrapper<T> load(AssetInfo& aInfo) {
		return T::load(aInfo);
	}

public:
	AssetInfo m_assetInfo;
};