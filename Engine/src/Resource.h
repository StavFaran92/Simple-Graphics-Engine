#pragma once

#include <cstdint>
#include <atomic>

class Texture;
class Mesh;
#include "Engine.h"
#include "MemoryPool.h"
#include "ResourceManager.h"

template<typename T>
class Resource
{
public:
	static Resource<T> empty;

	Resource() : m_uid(EMPTY_UUID) {};

	Resource(std::nullptr_t) : m_uid(EMPTY_UUID) {};

	Resource(UUID uid) : m_uid(uid) 
	{
		Engine::get()->getResourceManager()->incRef(uid);
	};

	Resource(const Resource<T>& other) 
	{
		m_uid = other.m_uid;
		if (other.m_uid != EMPTY_UUID)
		{
			Engine::get()->getResourceManager()->incRef(other.m_uid);
		}
	};

	Resource<T>& operator=(const Resource<T>& other)
	{
		if (m_uid == other.m_uid) 
			return *this;

		clean();

		m_uid = other.m_uid;
		if (other.m_uid != EMPTY_UUID)
		{
			Engine::get()->getResourceManager()->incRef(other.m_uid);
		}

		return *this;
	};

	Resource(Resource<T>&& other)
	{
		m_uid = other.m_uid;
		other.m_uid = EMPTY_UUID;
	};

	Resource<T>& operator=(Resource<T>&& other)
	{
		m_uid = other.m_uid;
		other.m_uid = EMPTY_UUID;

		return *this;
	};

	T* operator->()
	{
		return get();
	}

	inline T* get() const
	{
		m_cache = Engine::get()->getMemoryPool<T>()->get(m_uid);
		return m_cache;
	}

	inline UUID getUID() const 
	{ 
		return m_uid; 
	}

	void release()
	{
		clean();
	}

	bool isEmpty() const
	{
		return m_uid == EMPTY_UUID;
	}

	template <class Archive>
	void save(Archive& archive) const {
		archive(m_uid);
	}

	template <class Archive>
	void load(Archive& archive) {
		archive(m_uid);
		int a = Engine::get()->getResourceManager()->incRef(m_uid);
		logDebug("Loaded Resource: m_uid: " + m_uid + ", ref count: " + std::to_string(a));
		
	}

	~Resource<T>() // destructor
	{
		if(m_uid != EMPTY_UUID) clean();
	}

private:
	template<typename T>friend class Factory;
	

	void clean()
	{
		if (Engine::get()->getResourceManager()->decRef(m_uid) == 0)
		{
			if (m_uid != EMPTY_UUID)
			{
				Engine::get()->getMemoryPool<T>()->erase(m_uid);
			}

			m_uid = EMPTY_UUID;
		}
	}
private:
	UUID m_uid = EMPTY_UUID;
	mutable T* m_cache = nullptr;
};

template<typename T>
inline Resource<T> Resource<T>::empty;