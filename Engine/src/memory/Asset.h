#pragma once

#include <string>

#include "Assets.h"
#include "AssetFactory.h"

class Asset
{
public:
	static Asset empty;

	Asset() = default;

	Asset(UUID uuid) : uuid(uuid)
	{
		m_resource = resourceInner();
	};

	Asset(const Asset& other) : Asset(other.uuid)
	{
	};

	ResourceWrapper<Resource> resourceInner() const
	{
		return info().resource;
	}

	void erase()
	{
		Engine::get()->getSubSystem<Assets>()->deleteAsset(*this);
	}

	void makeDirty()
	{
		Engine::get()->getSubSystem<Assets>()->makeDirty(uuid);
	}

	UUID getUID() const
	{
		return uuid;
	}

	bool isEmpty() const
	{
		return resourceInner().isEmpty();
	}

	const AssetInfo& info() const
	{
		return Engine::get()->getSubSystem<Assets>()->getInfo(uuid);
	}

	void reimportAsset()
	{
		Engine::get()->getSubSystem<Assets>()->reimportAsset(uuid);
	}

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(uuid);
	}

private:
	UUID uuid = EMPTY_UUID;

	// Used mainly for debug
	ResourceWrapper<Resource> m_resource = ResourceWrapper<Resource>::empty;
private:
	template<typename T>friend class Factory;
	friend class Assets;
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
	virtual bool copyFiles(const std::string& fileLocation, AssetInfo&) = 0;
	virtual ResourceWrapper<Resource> load(AssetInfo& aInfo) = 0;
	virtual void save(Asset asset, const AssetInfo& aInfo) {};
	virtual std::string getRecommendedExtension(const AssetInfo& aInfo) { return getExtensionFromType(aInfo.aType); }; // Default behaviour, can be overriden if needed
};