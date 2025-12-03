#pragma once

#include "entt/entt.hpp"
#include "core/Core.h"

class Entity;

class EngineAPI SGE_Regsitry
{
public:
	entt::registry& get();

	entt::registry& getRegistry();
	const entt::registry& getRegistry() const;

	Entity createEntity();

	void removeEntity(const Entity& e);
private:

	entt::registry m_registry;
};