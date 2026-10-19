#pragma once

#include "sge.h"

#include <functional>
#include <string>
#include <vector>

// One entry of the Hierarchy's Create menu
struct EntityPreset
{
	std::string path;         // "Environment/Water/Pool": submenus, then the item
	std::string description;  // What it creates, shown to the agent
	std::function<Entity()> create;

	// Last part of the path: the menu label and create_preset's key
	std::string name() const;
};

const std::vector<EntityPreset>& entityPresets();
const EntityPreset* findPreset(const std::string& name);
