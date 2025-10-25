#pragma once

#include "memory/ResourceWrapper.h"
#include "core/Core.h"
#include "memory/AssetInfo.h"

template<typename T>
class AssetWrapper
{ 
public:
	static AssetWrapper<T> empty;

	AssetWrapper() = default;

	AssetWrapper(UUID uuid) : uuid(uuid)
	{
		m_resource = resource();
	};

	template<typename U/*, typename = std::enable_if_t<std::is_convertible_v<T*, U*>>*/>
	AssetWrapper<U> as() const
	{
		return AssetWrapper<U>(uuid);
	}

	// Upcast (texture -> asset)
	template<typename U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
	AssetWrapper(const AssetWrapper<U>& other)
	{
		uuid = other.getUID();
		m_resource = resource();
	}

	const ResourceWrapper<T> resource() const
	{
		return info().resource.as<T>();
	}

	ResourceWrapper<T> resource()
	{
		return info().resource.as<T>();
	}

	UUID getUID() const
	{
		return uuid;
	}

	bool isEmpty() const
	{
		return resource().isEmpty();
	}

	const AssetInfo& info() const
	{
		return Engine::get()->getSubSystem<Assets>()->getAsset(uuid);
	}

	inline T* get() const
	{
		return resource().get();
	}

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(uuid);
	}

private:
	UUID uuid = EMPTY_UUID;

	// Used mainly for debug
	ResourceWrapper<T> m_resource = ResourceWrapper<T>::empty;
private:
	template<typename T>friend class Factory;
	friend class Assets;

	//static AssetWrapper<T> promoteToAsset(const ResourceWrapper<T>& resource)
	//{
	//	return AssetWrapper<T>(resource.getUID());
	//}
};

template<typename T>
inline AssetWrapper<T> AssetWrapper<T>::empty;