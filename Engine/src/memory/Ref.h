#pragma once

template<typename T>
using Ref = std::shared_ptr<T>;

template<typename T, typename... Args>
Ref<T> createRef(Args&&... args)
{
    return std::make_shared<T>(std::forward<Args>(args)...);
}