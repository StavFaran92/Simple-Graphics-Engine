#pragma once

#include <string>

#include "Assets.h"
#include "AssetFactory.h"

struct BaseAssetParameters
{
	std::string name;

	virtual ~BaseAssetParameters() = default;
};

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
	static void convertAssetLoadParamsToAssetInfo(const std::string& fileLocation, const BaseAssetParameters& params, AssetInfo& aInfo);
	static Resource<T> load(AssetInfo& aInfo);
	static void save(AssetInfo& aInfo, const Resource<T>& asset);
};

class Asset
{
public:
	Asset() = default;
	virtual ~Asset() = default;

	static Resource<Asset> import(const std::string& fileLocation, const BaseAssetParameters& params);

	//virtual Resource<Asset> load(AssetInfo aInfo) = 0;

	// CRTP
	template <typename T>
	static Resource<T> import(const std::string& fileLocation, const BaseAssetParameters& params) {
		return T::import(fileLocation, params);
	}

	template <typename T>
	static Resource<T> load(AssetInfo& aInfo) {
		return T::load(aInfo);
	}
};