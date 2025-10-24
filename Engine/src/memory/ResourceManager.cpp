#include "memory/ResourceManager.h"

std::string ResourceManager::getRootDir() const
{
	return m_rootResourceDir;
}

void ResourceManager::setRootDir(const std::string& rootDir)
{
	m_rootResourceDir = rootDir;
}

int ResourceManager::getRefCount(ResourceID id) const
{
    auto iter = m_resourceRefCount.find(id);
    if (iter != m_resourceRefCount.end())
    {
        return iter->second;
    }
    return 0;
}

int ResourceManager::incRef(ResourceID id)
{
    return ++m_resourceRefCount[id];
}

int ResourceManager::decRef(ResourceID id)
{
    return --m_resourceRefCount[id];
}