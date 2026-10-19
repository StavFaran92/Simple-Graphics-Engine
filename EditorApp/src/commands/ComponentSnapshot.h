#pragma once

#include "runtime/Entity.h"
#include "serialize/Archiver.h"

#include <string>
#include <vector>

// Registry name of a component type, e.g. "PhysicsComponent"
template<typename T>
std::string componentTypeName()
{
	return Archiver::componentTypeName(std::string(entt::type_name<T>().value()));
}

// One component's state on one entity
struct ComponentSnapshot
{
	entity_id entity = 0;
	std::string component;  // "PhysicsComponent"
	bool present = false;   // false = the entity doesn't have this component
	std::string data;       // JSON from Archiver::saveComponent, empty when not present
};

// The entity with this id in the active scene (check valid())
Entity entityFromId(entity_id id);

ComponentSnapshot takeSnapshot(Entity e, const std::string& component);

// Snapshots of every component the entity has
std::vector<ComponentSnapshot> takeEntitySnapshot(Entity e);

// Makes the component match the snapshot: sets it, or removes it if it wasn't present
void applySnapshot(const ComponentSnapshot& snapshot);
