#pragma once

#include "memory/UUID.h"
#include "memory/ResourceManager.h"
#include "memory/MemoryManager.h"

template<typename T> class ResourceWrapper;

template<typename T>
class Factory
{
public:
    template<typename... Args>
    static ResourceWrapper<T> create(Args&&... args)
    {
        T* asset = new T(args...);
        auto uid = UUID::generate_uuid_v4();
        Engine::get()->getMemoryPool().add(uid, asset);
        //Engine::get()->getResourceManager()->incRef(uid);
        ResourceWrapper<T> res(uid);
        return res;
    }

    template<typename... Args>
    static ResourceWrapper<T> createUsingCustomUUID(UUID uid, Args&&... args)
    {
        T* asset = new T(args...);
        Engine::get()->getMemoryPool().add(uid, asset);
        //Engine::get()->getResourceManager()->incRef(uid);
        ResourceWrapper<T> res(uid);
        return res;
    }
};


