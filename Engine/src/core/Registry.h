#pragma once

#include <functional>

#include "entt/entt.hpp"
#include "core/Core.h"
#include "serialize/CerealHelpers.h"

struct Component;
class Entity;

class EngineAPI SGE_Regsitry
{
public:
	using Callback = std::function<void(Component&)>;
	using OnComponentEraseCallback = std::function<void()>;

	entt::registry& get();

	entt::registry& getRegistry();
	const entt::registry& getRegistry() const;

	Entity createEntity();

	void registerOnComponentAdded(const Callback& callback)
	{
		m_onComponentAddedCallback = callback;
	}

	void registerOnComponentRemoved(const OnComponentEraseCallback& callback)
	{
		m_onComponentRemovedCallback = callback;
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

	template<typename T>
	void removeComponent(entt::entity e)
	{
		m_registry.remove<T>(e);
		if (m_onComponentRemovedCallback)
		{
			m_onComponentRemovedCallback();
		}
	}

	void removeEntity(const Entity& e);
private:
	void invokeOnComponentAdded(Component& c)
	{
		if (m_onComponentAddedCallback)
		{
			m_onComponentAddedCallback(c);
		}
	}

	entt::registry m_registry;
	Callback m_onComponentAddedCallback;
	OnComponentEraseCallback m_onComponentRemovedCallback;
};