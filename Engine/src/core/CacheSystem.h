#pragma once

#include <memory>
#include <string>
#include <functional>
template<class T>  class ResourceWrapper;

#include "memory/MemoryPool.h"
#include "serialize/ProjectAssetRegistry.h"
#include "runtime/Context.h"
#include "core/Engine.h"
#include "memory/ResourceManager.h"

class Texture;
class Mesh;
class Shader;

/**
 * This class is reponsible on holding all the shared heavy resources (Textures, Shaders, etc..) in memory.
 * It is used to optimize the system by caching the resources in memory until they are no longer needed.
 */
class CacheSystem
{
public:
	CacheSystem() = default;
	CacheSystem(const std::unordered_map<std::string, UUID>& associations);
	~CacheSystem() = default;

	template<typename T>
	ResourceWrapper<T> createOrGetCached(const std::string& resourceName, const std::function<ResourceWrapper<T>()>& creationCallback)
	{
		auto it = m_associations.find(resourceName);
		if (it != m_associations.end())
		{
			UUID uid = it->second;
			return ResourceWrapper<T>(uid);
		}

		ResourceWrapper<T>& resource = creationCallback();
		m_associations[resourceName] = resource.getUID();

		//Engine::get()->getContext()->getProjectAssetRegistry()->addAssociation(resourceName, resource.getUID());

		return resource;
	}

	template<typename T>
	ResourceWrapper<T> get(const std::string& resourceName)
	{
		auto it = m_associations.find(resourceName);
		if (it != m_associations.end())
		{
			UUID uid = it->second;
			return ResourceWrapper<T>(uid);
		}

		return ResourceWrapper<T>::empty;
	}

	UUID getAssociation(const std::string& name) const
	{
		auto it = m_associations.find(name);
		if (it != m_associations.end())
		{
			return it->second;
		}
		return EMPTY_UUID;
	}

	std::string getName(UUID uuid) const
	{
		// Todo optimize
		for (auto& assc : m_associations)
		{
			if (assc.second == uuid)
			{
				return assc.first;
			}
		}
		return "N/A";
	}

	void addAssociation(const std::string& name, UUID uid)
	{
		m_associations[name] = uid;
		Engine::get()->getContext()->getProjectAssetRegistry()->addAssociation(name, uid);

	}

	void removeAssociation(const std::string& name)
	{
		m_associations.erase(name);
		Engine::get()->getContext()->getProjectAssetRegistry()->removeAssociation(name);

	}

	void clear();
private:
	std::unordered_map<std::string, UUID> m_associations;
};

