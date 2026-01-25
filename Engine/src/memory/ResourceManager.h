#pragma once

#include <string>
#include <unordered_map>

#include "core/Configurations.h"
#include "memory/ResourceWrapper.h"

class EngineAPI ResourceManager
{
public:
	std::string getRootDir() const;
	void setRootDir(const std::string& rootDir);

    int getRefCount(ResourceID uuid) const;


    int incRef(ResourceID uuid);

    int decRef(ResourceID uuid);

    ResourceWrapper<Resource> getResource(ResourceID) const;

    ResourceWrapper<Resource> loadResource(const std::string& fileLocation, ResourceLoadDescriptor& desc);



private:
	std::string m_rootResourceDir;

    std::unordered_map<ResourceID, ResourceWrapper<Resource>> m_resources;
};