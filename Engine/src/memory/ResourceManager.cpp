#include "memory/ResourceManager.h"

std::string ResourceManager::getRootDir() const
{
	return m_rootResourceDir;
}

void ResourceManager::setRootDir(const std::string& rootDir)
{
	m_rootResourceDir = rootDir;
}

int ResourceManager::getRefCount(UUID uuid) const
{
    auto iter = m_resourceRefCount.find(uuid);
    if (iter != m_resourceRefCount.end())
    {
        return iter->second;
    }
    return 0;
}

int ResourceManager::incRef(UUID uuid)
{
    return ++m_resourceRefCount[uuid];
}

int ResourceManager::decRef(UUID uuid)
{
    return --m_resourceRefCount[uuid];
}