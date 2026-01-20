#pragma once

#include "core/Engine.h"
#include "memory/MemoryPool.h"
#include "memory/ResourceManager.h"
#include "core/Configurations.h"

template<typename T>
class ResourceWrapper
{
public:
	static ResourceWrapper<T> empty;
	static const ResourceID emptyID = 0;

	void init()
	{
		if (id != 0)
		{
			Engine::get()->getResourceManager()->incRef(id);
			m_cache = Engine::get()->getMemoryPool().get(id);
		}
	}

	ResourceWrapper() : id(emptyID) {};

	ResourceWrapper(std::nullptr_t) : id(emptyID) {};

	explicit ResourceWrapper(ResourceID id) : id(id)
	{
		init();
	};

	// Copy constructor
	ResourceWrapper(const ResourceWrapper<T>& other) 
	{
		id = other.id;
		init();
	};

	// Copy assignemnt operator
	ResourceWrapper<T>& operator=(const ResourceWrapper<T>& other)
	{
		if (id == other.id) 
			return *this;

		clean();

		id = other.id;
		init();

		return *this;
	};

	// Move constructor
	ResourceWrapper(ResourceWrapper<T>&& other)
	{
		id = other.id;
		m_cache = other.m_cache;

		other.id = emptyID;
		other.m_cache = nullptr;
	};

	// Move Assignment operator
	ResourceWrapper<T>& operator=(ResourceWrapper<T>&& other) noexcept
	{
		clean();
		id = other.id;
		m_cache = other.m_cache;

		other.id = emptyID;
		other.m_cache = nullptr;

		return *this;
	};

	T* operator->()
	{
		return get();
	}

	inline T* get() const
	{
		return static_cast<T*>(Engine::get()->getMemoryPool().get(id));
	}

	// TODO reenforce

	//const T* operator->() const
	//{
	//	return get();
	//}

	//T* operator->()
	//{
	//	return get();
	//}

	//const T* get() const
	//{
	//	return static_cast<T*>(Engine::get()->getMemoryPool().get(uuid));
	//}

	//const T* get() const
	//{
	//	return static_cast<T*>(Engine::get()->getMemoryPool().get(uuid));
	//}

	inline ResourceID getUID() const 
	{ 
		return id; 
	}

	bool isEmpty() const
	{
		return id == emptyID;
	}

	~ResourceWrapper<T>() // destructor
	{
		if(id != emptyID)
			clean();
	}

	// Upcast (texture -> asset)
	template<typename U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
	ResourceWrapper(const ResourceWrapper<U>& other) 
	{
		id = other.getUID();
		init();
	}

	// Downcast (Asset -> Texture)
	template<typename U/*, typename = std::enable_if_t<std::is_convertible_v<T*, U*>>*/>
	ResourceWrapper<U> as() const
	{
		return ResourceWrapper<U>(id);
	}

private:
	template<typename T>friend class Factory;

	void clean()
	{
		if (Engine::get()->getResourceManager()->decRef(id) == 0)
		{
			if (id != emptyID)
			{
				Engine::get()->getMemoryPool().erase(id);
			}

			id = emptyID;
		}
	}
protected:
	ResourceID id = emptyID;
	mutable ResourceBase* m_cache = nullptr;
};

template<typename T>
inline ResourceWrapper<T> ResourceWrapper<T>::empty;