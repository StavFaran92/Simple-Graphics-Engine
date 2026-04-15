#include "memory/ResourceManager.h"

#include "core/Logger.h"

std::mutex cacheMutex;

std::string ResourceManager::getRootDir() const
{
	return m_rootResourceDir;
}

void ResourceManager::setRootDir(const std::string& rootDir)
{
	m_rootResourceDir = rootDir;
}

//ResourceWrapper<Resource> ResourceManager::loadResource(const std::string& fileLocation, ResourceLoadDescriptor& desc)
//{
//	// Validate input
//	if (fileLocation.empty() || !std::filesystem::exists(fileLocation))
//	{
//		logError("Invalid asset path specified.");
//		return ResourceWrapper<Resource>::empty;
//	}
//
//	// Load
//	ResourceWrapper<Resource> resource = AssetFactory::getManager(desc.aType)->load(desc);
//	if (resource.isEmpty() || !resource.get())
//	{
//		logError("Failed to load file {}", fileLocation);
//		return ResourceWrapper<Resource>::empty;
//	}
//
//	return resource;
//}

ResourceRef<Resource> ResourceManager::getResource(ResourceID rid) const
{
    {
        std::scoped_lock lock(cacheMutex);

        auto it = m_resourceCache.find(rid);
        if (it != m_resourceCache.end())
        {
            if (auto existing = it->second)
            {
                return existing;
            }
        }

        return ResourceRef<Resource>::empty;
    }
}

ResourceRef<Resource> ResourceManager::createOrGetCached(ResourceID id, const std::function<ResourceRef<Resource>(void)>& creationCallback)
{
    {
        std::scoped_lock lock(cacheMutex);

        auto it = m_resourceCache.find(id);
        if (it != m_resourceCache.end())
        {
            if (auto existing = it->second)
            {
                return existing;
            }
        }
    }

    // Create outside the lock (important)
    ResourceRef<Resource> created = creationCallback();

    if (created.isEmpty())
    {
        return ResourceRef<Resource>::empty;
    }

    {
        std::scoped_lock lock(cacheMutex);

        // Another thread might have beaten us to it
        auto& slot = m_resourceCache[created.getUID()];
        if (auto existing = slot)
        {
            return existing;
        }

        slot = created;
    }

    return created;
}
