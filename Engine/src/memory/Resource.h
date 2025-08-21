#pragma once

#include <cstdint>
#include <atomic>

class Texture;
class Mesh;
#include "core/Engine.h"
#include "memory/MemoryPool.h"
#include "memory/ResourceManager.h"
#include "serialize/CerealHelpers.h"

template<typename T>
class Resource
{
public:
	static Resource<T> empty;

	Resource() : uuid(EMPTY_UUID) {};

	Resource(std::nullptr_t) : uuid(EMPTY_UUID) {};

	Resource(UUID uid) : uuid(uid) 
	{
		Engine::get()->getResourceManager()->incRef(uid);

		if(!isEmpty()) 
			m_cache = Engine::get()->getMemoryPool().get(uuid);
	};

	Resource(const Resource<T>& other) 
	{
		uuid = other.uuid;
		if (other.uuid != EMPTY_UUID)
		{
			Engine::get()->getResourceManager()->incRef(other.uuid);
		}

		if (!isEmpty()) 
			m_cache = Engine::get()->getMemoryPool().get(uuid);
	};

	Resource<T>& operator=(const Resource<T>& other)
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

	Resource(Resource<T>&& other)
	{
		uuid = other.uuid;
		other.uuid = EMPTY_UUID;
		if (!isEmpty()) 
			m_cache = Engine::get()->getMemoryPool().get(uuid);
	};

	Resource<T>& operator=(Resource<T>&& other)
	{
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
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(uuid);
	}

	~Resource<T>() // destructor
	{
		if(uuid != EMPTY_UUID) clean();
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
	mutable Asset* m_cache = nullptr;
};

inline Resource<Asset> Resource<Asset>::empty;