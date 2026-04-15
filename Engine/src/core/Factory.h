#pragma once

#include "memory/UUID.h"
#include "memory/ResourceManager.h"
#include "memory/MemoryManager.h"
#include "memory/ResourceBase.h"
#include "core/Engine.h"

template<typename T> class ResourceWrapper;

template<typename T>
class Factory
{
public:
    template<typename... Args>
    static ResourceWrapper<T> create(Args&&... args)
    {
        ResourceID id = Resource::getNewResourceID();
        return Engine::get()->getResourceManager()->create<T>(id, args...);

        //T* asset = new T(args...);
        //ResourceID id = Resource::getNewResourceID();
        //Engine::get()->getMemoryPool().add(id, asset);
        //ResourceWrapper<T> res = Resource::createNewResource<T>(id);
        //return res;
    }

};


