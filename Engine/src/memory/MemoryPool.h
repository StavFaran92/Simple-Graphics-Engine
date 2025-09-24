#pragma once

#include <unordered_map>
#include <memory>
#include <functional>
#include "memory/UUID.h"
#include "core/Logger.h"

template<typename T>
class MemoryPool
{
public:
    
    void add(UUID uid, T* resource)
    {
        m_memory[uid] = resource;
    }

    std::vector<std::string> getAll() const
    {
        std::vector<std::string> result;
        for (auto [uuid, _] : m_memory)
        {
            result.push_back(uuid);
        }
        return result;
    }

private:
    template<class T>friend class ResourceWrapper;
    T* get(UUID uid)
    {
        //static_assert(std::is_base_of<T, K>::value, "K must derive from T");

        auto iter = m_memory.find(uid);
        if (iter == m_memory.end())
        {
            logError("Could not locate resource: " + uid);
            return nullptr;
        }
        return iter->second;
    }

    void erase(UUID uid)
    {
        auto iter = m_memory.find(uid);
        if (iter == m_memory.end()) return;
        delete iter->second;
        m_memory.erase(uid);
    }
    std::unordered_map<UUID, T*> m_memory;
    

};