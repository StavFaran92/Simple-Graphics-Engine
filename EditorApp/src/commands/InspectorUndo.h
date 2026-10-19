#pragma once

#include "ComponentSnapshot.h"

#include <string>

// Inspector edits as undo steps: snapshot before a component's widgets run, record after.
// Frames of one interaction (a whole drag) merge into a single step.
namespace InspectorUndo
{
	ComponentSnapshot beforeWidgets(Entity e, const std::string& component);

	void afterWidgets(const ComponentSnapshot& before, const std::string& label);
}
