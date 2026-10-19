#include "InspectorUndo.h"

#include "CommandStack.h"
#include "EntityCommands.h"

#include "sge.h"
#include "imgui.h"

#include <map>
#include <memory>
#include <utility>

namespace
{
	struct PendingEdit
	{
		ComponentSnapshot before;  // when the interaction started
		ComponentSnapshot after;   // latest frame
		std::string label;
	};

	// Interactions still in progress, by entity + component
	std::map<std::pair<entity_id, std::string>, PendingEdit> s_pending;

	void recordFinishedEdits()
	{
		// While an item is active the user is still dragging or typing
		if (ImGui::IsAnyItemActive())
			return;

		for (auto& [_, edit] : s_pending)
		{
			CommandStack::Instance().record(std::make_unique<EditComponentsCmd>(
				std::vector<ComponentSnapshot>{ edit.before }, std::vector<ComponentSnapshot>{ edit.after }, edit.label));
		}
		s_pending.clear();
	}
}

ComponentSnapshot InspectorUndo::beforeWidgets(Entity e, const std::string& component)
{
	// Scripts change components every frame while the game runs; those aren't edits
	if (Engine::get()->getContext()->getActiveScene()->isSimulationActive())
		return {};

	return takeSnapshot(e, component);
}

void InspectorUndo::afterWidgets(const ComponentSnapshot& before, const std::string& label)
{
	// An empty name means beforeWidgets didn't track this frame
	if (!before.component.empty())
	{
		ComponentSnapshot after = takeSnapshot(entityFromId(before.entity), before.component);
		if (after.present != before.present || after.data != before.data)
		{
			const auto key = std::make_pair(before.entity, before.component);
			auto it = s_pending.find(key);
			if (it == s_pending.end())
				s_pending.emplace(key, PendingEdit{ before, after, label });
			else
				it->second.after = after;
		}
	}

	recordFinishedEdits();
}
