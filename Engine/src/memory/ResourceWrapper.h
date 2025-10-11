#pragma once

#include <cstdint>
#include <atomic>

#include "core/Engine.h"
#include "memory/MemoryPool.h"
#include "memory/ResourceManager.h"
#include "serialize/CerealHelpers.h"

template<typename T>
class ResourceWrapper
{
public:
	static ResourceWrapper<T> empty;

	ResourceWrapper() : uuid(EMPTY_UUID) {};

	ResourceWrapper(std::nullptr_t) : uuid(EMPTY_UUID) {};

	ResourceWrapper(UUID uid) : uuid(uid) 
	{
		Engine::get()->getResourceManager()->incRef(uid);

		if(!isEmpty()) 
			m_cache = Engine::get()->getMemoryPool().get(uuid);
	};

	ResourceWrapper(const ResourceWrapper<T>& other) 
	{
		uuid = other.uuid;
		if (other.uuid != EMPTY_UUID)
		{
			Engine::get()->getResourceManager()->incRef(other.uuid);
		}

		if (!isEmpty()) 
			m_cache = Engine::get()->getMemoryPool().get(uuid);
	};

	ResourceWrapper<T>& operator=(const ResourceWrapper<T>& other)
	{
		if (uuid == other.uuid) 
			return *this;

		clean();

		uuid = other.uuid;
		if (other.uuid != EMPTY_UUID)
		{
			Engine::get()->getResourceManager()->incRef(other.uuid);
		}
		if (!isEmpty()) 
			m_cache = Engine::get()->getMemoryPool().get(uuid);

		return *this;
	};

	ResourceWrapper(ResourceWrapper<T>&& other)
	{
		uuid = other.uuid;
		other.uuid = EMPTY_UUID;
		if (!isEmpty()) 
			m_cache = Engine::get()->getMemoryPool().get(uuid);
	};

	ResourceWrapper<T>& operator=(ResourceWrapper<T>&& other) noexcept
	{
		clean();
		uuid = other.uuid;
		other.uuid = EMPTY_UUID;
		if (!isEmpty()) 
			m_cache = Engine::get()->getMemoryPool().get(uuid);

		return *this;
	};

	T* operator->()
	{
		return get();
	}

	inline T* get() const
	{
		return static_cast<T*>(Engine::get()->getMemoryPool().get(uuid));
	}

	inline UUID getUID() const 
	{ 
		return uuid; 
	}

	void release()
	{
		clean();
	}

	bool isEmpty() const
	{
		return uuid == EMPTY_UUID;
	}

	template <class Archive>
	void save(Archive& archive) const {
		archive(CEREAL_NVP(uuid));
	}

	template <class Archive>
	void load(Archive& archive) {
		archive(CEREAL_NVP(uuid));

		Engine::get()->getResourceManager()->incRef(uuid);

		if (!isEmpty())
			m_cache = Engine::get()->getMemoryPool().get(uuid);
	}

	~ResourceWrapper<T>() // destructor
	{
		if(uuid != EMPTY_UUID) clean();
	}

	// Upcast (texture -> asset)
	template<typename U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
	ResourceWrapper(const ResourceWrapper<U>& other) 
	{
		uuid = other.getUID();
		if (other.getUID() != EMPTY_UUID)
		{
			Engine::get()->getResourceManager()->incRef(other.getUID());
		}

		if (!isEmpty())
			m_cache = Engine::get()->getMemoryPool().get(uuid);
	}

	// Downcast (Asset -> Texture)
	template<typename U/*, typename = std::enable_if_t<std::is_convertible_v<T*, U*>>*/>
	ResourceWrapper<U> as() const
	{
		return ResourceWrapper<U>(uuid);
	}

	bool isAsset() const
	{
		return m_isAsset;
	}

private:
	template<typename T>friend class Factory;
	

	void clean()
	{
		if (Engine::get()->getResourceManager()->decRef(uuid) == 0)
		{
			if (uuid != EMPTY_UUID)
			{
				Engine::get()->getMemoryPool().erase(uuid);
			}

			uuid = EMPTY_UUID;
		}
	}
private:
	UUID uuid = EMPTY_UUID;
	mutable ResourceBase* m_cache = nullptr;
	bool m_isAsset = false;
};

template<typename T>
inline ResourceWrapper<T> ResourceWrapper<T>::empty;