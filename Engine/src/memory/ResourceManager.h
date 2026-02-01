#pragma once

#include <string>
#include <unordered_map>

#include "core/Configurations.h"
#include "memory/ResourceWrapper.h"
#include "memory/ResourceView.h"
#include "core/Logger.h"

class EngineAPI ResourceManager
{
public:
	std::string getRootDir() const;
	void setRootDir(const std::string& rootDir);

    int getRefCount(ResourceID uuid) const;


    int incRef(ResourceID uuid);

    int decRef(ResourceID uuid);

    ResourceWrapper<Resource> getResource(ResourceID) const;

    //ResourceWrapper<Resource> loadResource(const std::string& fileLocation, ResourceLoadDescriptor& desc);

    ResourceWrapper<Resource> createOrGetCached(ResourceID id, const std::function<ResourceWrapper<Resource>(void)>& creationCallback);

    template<typename T, typename... Args>
    ResourceWrapper<T> create(ResourceID id, Args&&... args)
    {
        static_assert(std::is_base_of_v<Resource, T>);

        auto obj = std::shared_ptr<T>(new T(args...),
            [id](T* p)
            {
                // your custom deletion logic
                logDebug("Erase resource: {}", id);
                delete p;
            });
        auto& resource = ResourceWrapper<T>(obj, id);

        logDebug("Create new resource: {}", id);

        m_resourceCache[id] = ResourceView<Resource>(resource);

        return resource;
    }







private:
	std::string m_rootResourceDir;

    std::unordered_map<ResourceID, ResourceView<Resource>> m_resourceCache; // todo - should be weak ref
};