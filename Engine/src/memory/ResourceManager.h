#pragma once

#include <string>
#include <unordered_map>

#include "core/Configurations.h"
#include "memory/ResourceView.h"
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

        auto obj = std::shared_ptr<T>(
            new T(std::forward<Args>(args)...),
            [id](T* p) {
                // custom delete logic
                Trace::destroyResource(id, typeid(T).name());
                delete p;
            }
        );
        auto& resource = ResourceRef<T>(obj, id);

        logDebug("Create new resource: {}", id);

        Trace::createResource(id, typeid(T).name());

        m_resourceCache[id] = obj;

        return resource;
    }

private:
	std::string m_rootResourceDir;

    std::unordered_map<ResourceID, std::weak_ptr<Resource>> m_resourceCache; // todo - should be weak ref
};