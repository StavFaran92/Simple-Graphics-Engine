#pragma once

#include <unordered_map>
#include <memory>
#include <functional>
#include "core/Logger.h"
#include "core/Configurations.h"

template<typename T>
class MemoryPool
{
public:
    
    void add(ResourceID id, T* resource)
    {
        m_memory[id] = resource;

        logDebug("Added Resource {} to Memory", id);
    }

    std::vector<std::string> getAll() const
    {
        std::vector<std::string> result;
        for (auto [id, _] : m_memory)
        {
            result.push_back(id);
        }
        return result;
    }

private:
    template<class T>friend class ResourceRef;
    template<class T>friend class AssetRef;

    T* get(ResourceID id)
    {
        //static_assert(std::is_base_of<T, K>::value, "K must derive from T");

        auto iter = m_memory.find(id);
        if (iter == m_memory.end())
        {
            logError("Could not locate resource: {} in memory", id);
            return nullptr;
        }
        return iter->second;
    }

    void erase(ResourceID id)
    {
        auto iter = m_memory.find(id);
        if (iter == m_memory.end()) return;
        delete iter->second;
        m_memory.erase(id);

        logDebug("Erased Resource {} from memory", id);
    }

    int getRefCount(ResourceID id) const
    {
        auto iter = m_resourceRefCount.find(id);
        if (iter != m_resourceRefCount.end())
        {
            return iter->second;
        }
        return 0;
    }

    int incRef(ResourceID id)
    {
        return ++m_resourceRefCount[id];
    }

    int decRef(ResourceID id)
    {
        return --m_resourceRefCount[id];
    }

    std::unordered_map<ResourceID, T*> m_memory;

    std::unordered_map<ResourceID, int> m_resourceRefCount;
    

};