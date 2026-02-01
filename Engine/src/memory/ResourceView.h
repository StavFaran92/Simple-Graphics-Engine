#pragma once

#include "memory/ResourceWrapper.h"

#include <memory>

template<typename T>
class ResourceView {
public:
    ResourceView() = default;
    ResourceView(const std::shared_ptr<T>& ptr)
        : m_weak(ptr) {
    }

    bool isValid() const noexcept {
        return !m_weak.expired();
    }

    ResourceWrapper<T> lock() const {
        if(!isValid()) return ResourceWrapper<T>::empty;

        std::shared_ptr<T> sptr = m_weak.lock();
        
    }

private:
    std::weak_ptr<T> m_weak;
};