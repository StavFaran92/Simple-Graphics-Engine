#pragma once

#include <string>

#include "Assets.h"
#include "AssetFactory.h"

class ImportSettings
{

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

class Asset
{
public:
	Asset() = default;
	virtual ~Asset() = default;

	//virtual Resource<Asset> import(const std::string& fileLocation, const ImportSettings& settings) = 0;

	//virtual Resource<Asset> load(AssetInfo aInfo) = 0;

	// CRTP
	template <typename T>
	static Resource<T> import(const std::string& fileLocation, const ImportSettings& settings) {
		return T::import(fileLocation, settings);
	}

	template <typename T>
	static Resource<T> load(AssetInfo aInfo) {
		return T::load(aInfo);
	}
};