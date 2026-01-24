#pragma once

#include <string>

#include "memory/AssetHandle.h"
#include "Assets.h"
#include "AssetFactory.h"

class Asset
{
public:
	ResourceWrapper<Resource> resourceInner() const
	{
		return m_resource;
	}

protected:
	ResourceWrapper<Resource> m_resource;
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

class AssetManager {
public:
	virtual bool copyFiles(const std::string& fileLocation, AssetRecord&) = 0;
	virtual ResourceWrapper<Resource> load(AssetRecord& aInfo) = 0;
	virtual void save(AssetHandle<Asset> asset, const AssetRecord& aInfo) {};
	virtual std::string getRecommendedExtension(const AssetRecord& aInfo) { return getExtensionFromType(aInfo.aType); }; // Default behaviour, can be overriden if needed
};