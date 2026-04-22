#pragma once

#include "memory/ResourceRef.h"

#include <memory>

template<typename T>
class ResourceView {
public:
    ResourceView() = default;
    ResourceView(const std::shared_ptr<T>& ptr)
        : m_weak(ptr) {
    }

    ResourceView(const ResourceRef<T>& ref)
        : m_weak(std::static_pointer_cast<T>(ref.m_resource)),
          m_id(ref.m_id) {
    }

    bool isValid() const noexcept {
        return !m_weak.expired();
    }

    ResourceRef<T> lock() const {
        if(!isValid()) return ResourceRef<T>::empty;

        std::shared_ptr<T> sptr = m_weak.lock();
        if (!sptr) return ResourceRef<T>::empty;

        return ResourceRef<T>(std::static_pointer_cast<Resource>(sptr), m_id);
    }

private:
    std::weak_ptr<T> m_weak;
    ResourceID m_id = ResourceRef<T>::emptyID;
};