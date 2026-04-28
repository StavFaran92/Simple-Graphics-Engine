#pragma once

#include "memory/ResourceRef.h"
#include "memory/AssetRecord.h"

class Asset;

class EngineAPI AssetHandleImpl
{
public:
	static const AssetRecord& getInfo(UUID uuid);

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
		m_cachedAsset = std::dynamic_pointer_cast<T>(info().asset);
	};

	AssetRef& operator=(const AssetRef& other)
	{
		if (uuid == other.uuid)
			return *this;

		uuid = other.uuid;
		m_cachedAsset = std::dynamic_pointer_cast<T>(info().asset);

		m_version++;

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
		m_cachedAsset = std::dynamic_pointer_cast<T>(info().asset);
		m_version = other.getVersion();
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
		assert(uuid == m_cachedAsset->getUID());
		return m_cachedAsset;
	}

	uint64_t getVersion() const
	{
		return m_version;
	}

	template <class Archive>
	void save(Archive& archive) const {
		SERIALIZED_MEMBER(uuid);
	}

	template <class Archive>
	void load(Archive& archive) {
		SERIALIZED_MEMBER(uuid);
		m_cachedAsset = std::dynamic_pointer_cast<T>(info().asset);
	}

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(AssetRef, uuid);

private:

	UUID uuid = EMPTY_UUID;

	uint64_t m_version = 1;

	std::function<void(UUID)> onChangedListener;

	mutable Ref<T> m_cachedAsset;

	// Used mainly for debug
	mutable ResourceRef<Resource> m_resource_DEBUG = ResourceRef<Resource>::empty;
	
private:
	template<typename T>friend class Factory;
	friend class Assets;
};

template<typename T>
inline AssetRef<T> AssetRef<T>::empty;

