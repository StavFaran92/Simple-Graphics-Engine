#pragma once

#include "ComponentSnapshot.h"
#include "AssetSnapshot.h"

#include <string>

// Inspector edits as undo steps: track before a component's or an asset's widgets run, commit after.
// Frames of one interaction (a whole drag) merge into a single step.
namespace InspectorUndo
{
	ComponentSnapshot trackComponent(Entity e, const std::string& component);

	void commitComponent(const ComponentSnapshot& before, const std::string& label);

	AssetSnapshot trackAsset(UUID asset);

	void commitAsset(const AssetSnapshot& before, const std::string& label);
}
