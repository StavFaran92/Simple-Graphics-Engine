#pragma once

#include <memory>
#include <type_traits>

#include "core/Configurations.h"
#include "memory/ResourceBase.h"

template<typename T>
class ResourceWrapper
{
public:
    static ResourceWrapper<T> empty;
    static constexpr ResourceID emptyID = 0;

    ResourceWrapper() = default;
    ResourceWrapper(std::nullptr_t) {}

    template<typename... Args>
    static ResourceWrapper<T> createResource(ResourceID id, Args&&... args)
    {
        static_assert(std::is_base_of_v<Resource, T>);

        auto resource = std::make_shared<T>(std::forward<Args>(args)...);
        return ResourceWrapper<T>(resource, id);
    }

    // Copy ctor
    ResourceWrapper(const ResourceWrapper&) = default;

    // Copy assign
    ResourceWrapper& operator=(const ResourceWrapper&) = default;

    // Move ctor
    ResourceWrapper(ResourceWrapper&&) noexcept = default;

    // Move assign
    ResourceWrapper& operator=(ResourceWrapper&&) noexcept = default;

    ~ResourceWrapper() = default;

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
    ResourceWrapper(const ResourceWrapper<U>& other)
        : m_resource(other.m_resource), m_id(other.m_id)
    {
    }

    // Downcast (Base -> Derived)
    template<typename U>
    ResourceWrapper<U> as() const
    {
        return ResourceWrapper<U>(m_resource, m_id);
    }

private:
    // Construct from shared_ptr
    explicit ResourceWrapper(std::shared_ptr<Resource> resource, ResourceID id)
        : m_resource(std::move(resource)), m_id(id)
    {
    }

    template<typename U> friend class ResourceWrapper;

    std::shared_ptr<Resource> m_resource;
    ResourceID m_id = emptyID;
};

template<typename T>
ResourceWrapper<T> ResourceWrapper<T>::empty{};