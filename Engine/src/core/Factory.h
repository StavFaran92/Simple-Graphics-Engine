#pragma once

#include "memory/UUID.h"
#include "memory/ResourceManager.h"

template<typename T> class Resource;

template<typename T>
class Factory
{
public:
    template<typename... Args>
    static Resource<T> create(Args&&... args)
    {
        T* asset = new T(args...);
        auto uid = uuid::generate_uuid_v4();
        Engine::get()->getMemoryPool<T>()->add(uid, asset);
        Engine::get()->getResourceManager()->incRef(uid);
        Resource<T> res(uid);
        return res;
    }

    template<typename... Args>
    static Resource<T> createUsingCustomUUID(UUID uid, Args&&... args)
    {
        T* asset = new T(args...);
        Engine::get()->getMemoryPool<T>()->add(uid, asset);
        Engine::get()->getResourceManager()->incRef(uid);
        Resource<T> res(uid);
        return res;
    }
};


