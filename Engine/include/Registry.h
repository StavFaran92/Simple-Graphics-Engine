#pragma once

#include "entt/entt.hpp"
#include "Core.h"

class Entity;

class EngineAPI SGE_Regsitry
{
public:
	entt::registry& get();

	entt::registry& getRegistry();
	const entt::registry& getRegistry() const;

	Entity createEntity();

	Entity createEntity(const std::string& name);

	void removeEntity(const Entity& e);
private:

	entt::registry m_registry;
};