#pragma once

#include "memory/ResourceWrapper.h"
#include "core/Core.h"

class AssetInfo;

template<typename T>
class AssetWrapper
{ 
public:
	static AssetWrapper<T> empty;

	AssetWrapper() = default;

	AssetWrapper(UUID uid) : m_resource(uid)
	{};

	template<typename U/*, typename = std::enable_if_t<std::is_convertible_v<T*, U*>>*/>
	AssetWrapper<U> as() const
	{
		return AssetWrapper<U>(m_resource.getUID());
	}

	// Upcast (texture -> asset)
	template<typename U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
	AssetWrapper(const AssetWrapper<U>& other)
	{
		m_resource = ResourceWrapper<T>(other.resource().getUID());
	}

	const ResourceWrapper<T>& resource() const
	{
		return m_resource;
	}

	ResourceWrapper<T>& resource()
	{
		return m_resource;
	}

	UUID getUID() const
	{
		return m_resource.getUID();
	}

	bool isEmpty() const
	{
		return m_resource.isEmpty();
	}

	const AssetInfo& info() const
	{
		//return m_assetInfo; TODO fix
	}

	inline T* get() const
	{
		return m_resource.get();
	}

	template <class Archive>
	void save(Archive& archive) const {
		archive(CEREAL_NVP(m_resource.getUID()));
	}

	template <class Archive>
	void load(Archive& archive) {
		UUID uuid;
		archive(CEREAL_NVP(uuid));

		m_resource = ResourceWrapper<T>(uuid);
	}

private:
	//AssetInfo m_assetInfo;

private:
	template<typename T>friend class Factory;
	friend class Assets;

	static AssetWrapper<T> promoteToAsset(const ResourceWrapper<T>& resource)
	{
		return AssetWrapper<T>(resource.getUID());
	}

	ResourceWrapper<T> m_resource = ResourceWrapper<T>::empty;
};

template<typename T>
inline AssetWrapper<T> AssetWrapper<T>::empty;