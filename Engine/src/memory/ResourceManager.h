#pragma once

#include <string>
#include <unordered_map>

#include "UUID.h"

class ResourceManager
{
public:
	std::string getRootDir() const;
	void setRootDir(const std::string& rootDir);

    int getRefCount(UUID uuid) const
    {
        auto iter = m_resourceRefCount.find(uuid);
        if (iter != m_resourceRefCount.end())
        {
            return iter->second;
        }
        return 0;
    }

    int incRef(UUID uuid)
    {
        return ++m_resourceRefCount[uuid];
    }

    int decRef(UUID uuid)
    {
        return --m_resourceRefCount[uuid];
    }

private:
	std::string m_rootResourceDir;

    std::unordered_map<UUID, int> m_resourceRefCount;
};