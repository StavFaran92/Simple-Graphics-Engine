#pragma once

#include "memory/ResourceWrapper.h"

#include <memory>

template<typename T>
class ResourceView {
public:
    ResourceView() = default;
    ResourceView(const ResourceWrapper<T>& ptr)
        : m_weak(ptr.m_resource) {
    }

    bool isValid() const noexcept 
    {
        return !m_weak.expired();
    }

    ResourceWrapper<T> lock() const 
    {
        if(!isValid()) return ResourceWrapper<T>::empty;

        std::shared_ptr<T> sptr = m_weak.lock();
        if (!sptr)
            return ResourceWrapper<T>::empty;

        return ResourceWrapper<T>(sptr, id);
    }

private:
    std::weak_ptr<T> m_weak;
    ResourceID id = 0;
};