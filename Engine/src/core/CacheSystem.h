#pragma once

#include <memory>
#include <string>

#include "memory/MemoryPool.h"
#include "serialize/ProjectAssetRegistry.h"
#include "runtime/Context.h"
#include "core/Engine.h"
#include "memory/ResourceManager.h"

/**
 * This class is reponsible on holding all the shared heavy resources (Textures, Shaders, etc..) in memory.
 * It is used to optimize the system by caching the resources in memory until they are no longer needed.
 */
class CacheSystem
{
public:
	CacheSystem(std::shared_ptr<ProjectAssetRegistry> par);
	~CacheSystem() = default;

	//template<typename T>
	//ResourceWrapper<T> get(const std::string& resourceName)
	//{
	//	auto it = m_associations.find(resourceName);
	//	if (it != m_associations.end())
	//	{
	//		UUID uid = it->second;
	//		return ResourceWrapper<T>(uid);
	//	}

	//	return ResourceWrapper<T>::empty;
	//}



	//std::string getName(UUID uuid) const
	//{
	//	// Todo optimize
	//	for (auto& assc : m_associations)
	//	{
	//		if (assc.second == uuid)
	//		{
	//			return assc.first;
	//		}
	//	}
	//	return "N/A";
	//}

	UUID getUUIDFromName(const std::string& name) const
	{
		auto it = m_nameToUUID.find(name);
		if (it != m_nameToUUID.end())
		{
			return it->second;
		}
		return EMPTY_UUID;
	}

	void addNameReference(const std::string& name, UUID uid)
	{
		m_nameToUUID[name] = uid;
		Engine::get()->getContext()->getProjectAssetRegistry()->syncNames(m_nameToUUID);

	}

	void removeNameReference(const std::string& name)
	{
		m_nameToUUID.erase(name);
		Engine::get()->getContext()->getProjectAssetRegistry()->syncNames(m_nameToUUID);
	}

	UUID getUUIDFromPath(const std::string& path) const
	{
		auto it = m_pathToUUID.find(path);
		if (it != m_pathToUUID.end())
		{
			return it->second;
		}
		return EMPTY_UUID;
	}

	void addPathReference(const std::string& path, UUID uid)
	{
		m_pathToUUID[path] = uid;
		Engine::get()->getContext()->getProjectAssetRegistry()->syncPaths(m_pathToUUID);

	}

	void removePathReference(const std::string& path)
	{
		m_pathToUUID.erase(path);
		Engine::get()->getContext()->getProjectAssetRegistry()->syncPaths(m_pathToUUID);
	}

	void clear();
private:
	std::unordered_map<std::string, UUID> m_pathToUUID;
	std::unordered_map<std::string, UUID> m_nameToUUID;
};

