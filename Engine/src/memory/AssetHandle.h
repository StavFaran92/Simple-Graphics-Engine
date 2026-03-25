#pragma once

#include "memory/ResourceWrapper.h"
#include "memory/AssetRecord.h"
#include "core/Engine.h"
#include "memory/ResourceManager.h"
#include "memory/Asset.h"

class EngineAPI Test
{
public:
	static ResourceWrapper<Resource> loadAssetResourceInternal(const AssetRecord& record, UUID uuid);

};

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
		m_cachedAsset = info().asset;
		//m_resource = resource();
	};

	AssetHandle& operator=(const AssetHandle& other)
	{
		if (uuid == other.uuid)
			return *this;

		uuid = other.uuid;

		m_cachedAsset = info().asset;

		notifyOnChanged();

		return *this;
	}

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

		m_cachedAsset = info().asset;
		//m_resource = resource();
	}

	ResourceWrapper<ResourceType> resource() const
	{
		if (isEmpty())
			return ResourceWrapper<ResourceType>::empty;

		const AssetRecord& record = info();

		auto resource = Engine::get()->getResourceManager()->createOrGetCached(
			record.resourceID,
			[this, &record]() {
				return Test::loadAssetResourceInternal(record, uuid);
			});

		m_resource_DEBUG = resource;

		return resource.as<ResourceType>();
	}

	void erase()
	{
		Engine::get()->getSubSystem<Assets>()->deleteAsset(uuid);
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
	}

	Ref<T> operator->()
	{
		return get();
	}

	Ref<T> operator->() const
	{
		return get();
	}

	inline Ref<T> get() const
	{
		if (!m_cachedAsset)
		{
			m_cachedAsset = info().asset;
		}
		return std::dynamic_pointer_cast<T>(m_cachedAsset);
	}

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(uuid);
	}

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(AssetHandle, uuid);


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

	mutable Ref<Asset> m_cachedAsset;

	// Used mainly for debug
	mutable ResourceWrapper<Resource> m_resource_DEBUG = ResourceWrapper<Resource>::empty;
	
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

