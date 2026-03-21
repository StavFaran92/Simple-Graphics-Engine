#pragma once

#include <functional>

#include "entt/entt.hpp"
#include "core/Core.h"

struct Component;
class Entity;

class EngineAPI SGE_Regsitry
{
public:
	using Callback = std::function<void(const Component&)>;

	entt::registry& get();

	entt::registry& getRegistry();
	const entt::registry& getRegistry() const;

	Entity createEntity();

	void registerOnComponentAdded(const Callback& callback)
	{
		m_onComponentAddedCallback = callback;
	}

	template<typename T>
	T& addComponent(entt::entity e, T* component)
	{
		T& addedComponent = m_registry.emplace_or_replace<T>(e, *component);
		invokeOnComponentAdded(addedComponent);
		return addedComponent;
	}

	template<typename T, typename... Args>
	T& addComponent(entt::entity e, Args&&... args)
	{
		T& addedComponent = m_registry.emplace_or_replace<T>(e, std::forward<Args>(args)...);
		invokeOnComponentAdded(addedComponent);
		return addedComponent;
	}

	void removeEntity(const Entity& e);
private:
	void invokeOnComponentAdded(const Component& c)
	{
		if (m_onComponentAddedCallback)
		{
			m_onComponentAddedCallback(c);
		}
	}

	entt::registry m_registry;
	Callback m_onComponentAddedCallback;
};