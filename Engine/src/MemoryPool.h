#pragma once

#include <unordered_map>
#include <memory>
#include <functional>
#include "UUID.h"
#include "Logger.h"

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
    template<class T>friend class Resource;
    T* get(UUID uid)
    {
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