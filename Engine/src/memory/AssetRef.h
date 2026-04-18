#pragma once

#include "memory/ResourceRef.h"
#include "memory/AssetRecord.h"

class Asset;

class EngineAPI AssetHandleImpl
{
public:
	static ResourceRef<Resource> loadAssetResourceInternal(const AssetRecord& record, UUID uuid);
	static const AssetRecord& getInfo(UUID uuid);
	static ResourceRef<Resource> createOrGetCachedResource(const AssetRecord& record, UUID uuid);
	static void syncAsset(UUID uuid);
	static void deleteAsset(UUID uuid);
	static void makeAssetDirty(UUID uuid);

};

template<typename T>
class AssetRef
{ 
	static_assert(std::is_base_of_v<Asset, T>,
		"AssetRef<T>: T must derive from Asset");
public:
	using ResourceType = typename T::ResourceType;

	static AssetRef<T> empty;

	AssetRef() = default;

	AssetRef(UUID uuid) : uuid(uuid)
	{
		get();
	};

	AssetRef& operator=(const AssetRef& other)
	{
		if (uuid == other.uuid)
			return *this;

		uuid = other.uuid;
		get();

		notifyOnChanged();

		return *this;
	}

	template<typename U/*, typename = std::enable_if_t<std::is_convertible_v<T*, U*>>*/>
	AssetRef<U> as() const
	{
		return AssetRef<U>(uuid);
	}

	// Upcast (texture -> asset)
	template<typename U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
	AssetRef(const AssetRef<U>& other)
	{
		uuid = other.getUID();
		get();
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
		if (isEmpty())
		{
			static AssetRecord emptyRecord;
			return emptyRecord;
		}

		return AssetHandleImpl::getInfo(uuid);
	}

	ResourceRef<ResourceType> resource() const
	{
		if (isEmpty())
			return ResourceRef<ResourceType>::empty;

		return get()->resource().as<ResourceType>();
	}

	void erase()
	{
		if (isEmpty())
			return;

		get()->deleteAsset();
	}

	void makeDirty()
	{
		if (isEmpty())
			return;

		get()->makeAssetDirty();
	}

	inline Ref<T> operator->()
	{
		return get();
	}

	inline Ref<T> operator->() const
	{
		return get();
	}

	inline Ref<T> get() const
	{
		if (!m_cachedAsset)
		{
			m_cachedAsset = std::dynamic_pointer_cast<T>(info().asset);
		}
		return m_cachedAsset;
	}

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(uuid);
	}

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(AssetRef, uuid);


	void registerOnChanged(std::function<void(UUID)> cb)
	{
		onChangedRegistry->listeners.push_back(std::move(cb));
	}

private:
	void notifyOnChanged()
	{
		if (onChangedRegistry->listeners.size() > 0)
		{
			for (auto& cb : onChangedRegistry->listeners)
			{
				cb(uuid);
			}
		}
	}
private:

	UUID uuid = EMPTY_UUID;

	struct OnChangedRegistry
	{
		std::vector<std::function<void(UUID)>> listeners;
	};

	std::shared_ptr<OnChangedRegistry> onChangedRegistry = std::make_shared<OnChangedRegistry>();

	mutable Ref<T> m_cachedAsset;

	// Used mainly for debug
	mutable ResourceRef<Resource> m_resource_DEBUG = ResourceRef<Resource>::empty;
	
private:
	template<typename T>friend class Factory;
	friend class Assets;
};

template<typename T>
inline AssetRef<T> AssetRef<T>::empty;

