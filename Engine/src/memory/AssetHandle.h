#pragma once

#include "memory/ResourceWrapper.h"
#include "memory/AssetRecord.h"
#include "core/Engine.h"
#include "memory/ResourceManager.h"

template<typename T>
class AssetHandle
{ 
	static_assert(std::is_base_of_v<Asset, T>,
		"AssetHandle<T>: T must derive from Asset");
public:
	using ResourceType = typename T::ResourceType;

	static AssetHandle<T> empty;

	AssetHandle() = default;

	AssetHandle(UUID uuid) : uuid(uuid)
	{
		//m_resource = resource();
	};

	template<typename U/*, typename = std::enable_if_t<std::is_convertible_v<T*, U*>>*/>
	AssetHandle<U> as() const
	{
		return AssetHandle<U>(uuid);
	}

	// Upcast (texture -> asset)
	template<typename U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
	AssetHandle(const AssetHandle<U>& other)
	{
		uuid = other.getUID();
		//m_resource = resource();
	}

	ResourceWrapper<ResourceType> resource() const
	{
		if (isEmpty()) return ResourceWrapper<ResourceType>::empty;

		const AssetRecord& record = info();
		return Engine::get()->getResourceManager()->createOrGetCached(record.resourceID, [this, &record]() {
			const Asset* asset = get();
			const AssetCreateDescriptor& createDesc = asset->getDescriptor();
			ResourceLoadDescriptor* loadDesc = createDesc.resourceDescriptor;

			asset->fillData(*loadDesc);
			loadDesc->sourcePath = record.sourcePath;

			ResourceWrapper<Resource> resource = loadDesc->createFunc();
			AssetRecord newRecord = record;
			newRecord.resourceID = resource.getUID();
			Engine::get()->getSubSystem<Assets>()->updateAsset(newRecord);
			return resource;
		}).as<ResourceType>();
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
		return uuid == EMPTY_UUID;
	}

	const AssetRecord& info() const
	{
		return Engine::get()->getSubSystem<Assets>()->getInfo(uuid);
	}

	void reimportAsset()
	{
		get()->reimportAsset();
	}

	T* operator->()
	{
		return get();
	}

	T* operator->() const
	{
		return get();
	}

	inline T* get() const
	{
		return static_cast<T*>(info().asset);
	}

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(uuid);
	}

private:
	UUID uuid = EMPTY_UUID;

	// Used mainly for debug
	//ResourceWrapper<Resource> m_resource = ResourceWrapper<Resource>::empty;
private:
	template<typename T>friend class Factory;
	friend class Assets;

	//static AssetHandle<T> promoteToAsset(const ResourceWrapper<T>& resource)
	//{
	//	return AssetHandle<T>(resource.getUID());
	//}
};

template<typename T>
inline AssetHandle<T> AssetHandle<T>::empty;