#pragma once

#include <string>
#include <unordered_map>

#include "core/Configurations.h"
#include "memory/ResourceRef.h"
#include "core/Logger.h"
#include "core/Trace.h"

class EngineAPI ResourceManager
{
public:
	std::string getRootDir() const;
	void setRootDir(const std::string& rootDir);

    ResourceRef<Resource> getResource(ResourceID) const;

    ResourceRef<Resource> createOrGetCached(ResourceID id, const std::function<ResourceRef<Resource>(void)>& creationCallback);

    template<typename T, typename... Args>
    ResourceRef<T> create(ResourceID id, Args&&... args)
    {
        static_assert(std::is_base_of_v<Resource, T>);

        auto obj = std::make_shared<T>(std::forward<Args>(args)...);
        auto& resource = ResourceRef<T>(obj, id);

        logDebug("Create new resource: {}", id);

        Trace::createNewResource(id, typeid(T).name());

        m_resourceCache[id] = resource;

        return resource;
    }

private:
	std::string m_rootResourceDir;

    std::unordered_map<ResourceID, ResourceRef<Resource>> m_resourceCache; // todo - should be weak ref
};