#pragma once

#include <string>
#include <unordered_map>

#include "core/Configurations.h"

class EngineAPI ResourceManager
{
public:
	std::string getRootDir() const;
	void setRootDir(const std::string& rootDir);

    int getRefCount(ResourceID uuid) const;


    int incRef(ResourceID uuid);

    int decRef(ResourceID uuid);

    //ResourceWrapper<Resource> getResource(ResourceID) const;



private:
	std::string m_rootResourceDir;

    std::unordered_map<ResourceID, int> m_resourceRefCount;

    //std::unordered_map<ResourceID, ResourceWrapper<Resource>> m_resources;
};