#pragma once

#include <memory>
#include <type_traits>

#include "core/Configurations.h"
#include "memory/ResourceBase.h"

template<typename T>
class ResourceRef
{
public:
    static ResourceRef<T> empty;
    static constexpr ResourceID emptyID = 0;

    ResourceRef() = default;
    ResourceRef(std::nullptr_t) {}

    // Copy ctor
    ResourceRef(const ResourceRef&) = default;

    // Copy assign
    ResourceRef& operator=(const ResourceRef&) = default;

    // Move ctor
    ResourceRef(ResourceRef&&) noexcept = default;

    // Move assign
    ResourceRef& operator=(ResourceRef&&) noexcept = default;

    ~ResourceRef() = default;

    // Access
    T* operator->()
    {
        return get();
    }

    const T* operator->() const
    {
        return get();
    }

    T* get() const
    {
        return m_resource
            ? static_cast<T*>(m_resource.get())
            : nullptr;
    }

    bool isEmpty() const
    {
        return !m_resource;
    }

    ResourceID getUID() const
    {
        return m_id;
    }

    explicit operator bool() const
    {
        return !isEmpty();
    }

    // Upcast (Derived -> Base)
    template<typename U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
    ResourceRef(const ResourceRef<U>& other)
        : m_resource(other.m_resource), m_id(other.m_id)
    {
    }

    // Downcast (Base -> Derived)
    template<typename U>
    ResourceRef<U> as() const
    {
        return ResourceRef<U>(m_resource, m_id);
    }

private:
    // Construct from shared_ptr
    explicit ResourceRef(std::shared_ptr<Resource> resource, ResourceID id)
        : m_resource(std::move(resource)), m_id(id)
    {
    }

    template<typename U> friend class ResourceRef;
    friend class ResourceManager;

    std::shared_ptr<Resource> m_resource;
    ResourceID m_id = emptyID;
};

template<typename T>
ResourceRef<T> ResourceRef<T>::empty{};