#include "InspectorUndo.h"

#include "CommandStack.h"
#include "EntityCommands.h"
#include "AssetCommands.h"

#include "sge.h"
#include "imgui.h"

#include <map>
#include <memory>
#include <utility>

namespace
{
	template<typename Snapshot>
	struct PendingEdit
	{
		Snapshot before;  // when the interaction started
		Snapshot after;   // latest frame
		std::string label;
	};

	// Interactions still in progress: components by entity + component, assets by uuid
	std::map<std::pair<entity_id, std::string>, PendingEdit<ComponentSnapshot>> s_pendingComponents;
	std::map<std::string, PendingEdit<AssetSnapshot>> s_pendingAssets;

	void recordFinishedEdits()
	{
		// While an item is active the user is still dragging or typing
		if (ImGui::IsAnyItemActive())
			return;

		for (auto& [_, edit] : s_pendingComponents)
		{
			CommandStack::Instance().record(std::make_unique<EditComponentsCmd>(
				std::vector<ComponentSnapshot>{ edit.before }, std::vector<ComponentSnapshot>{ edit.after }, edit.label));
		}
		s_pendingComponents.clear();

		for (auto& [_, edit] : s_pendingAssets)
			CommandStack::Instance().record(std::make_unique<EditAssetCmd>(edit.before, edit.after, edit.label));
		s_pendingAssets.clear();
	}
}

ComponentSnapshot InspectorUndo::trackComponent(Entity e, const std::string& component)
{
	// Scripts change components every frame while the game runs; those aren't edits
	if (Engine::get()->getContext()->getActiveScene()->isSimulationActive())
		return {};

	return takeSnapshot(e, component);
}

void InspectorUndo::commitComponent(const ComponentSnapshot& before, const std::string& label)
{
	// An empty name means trackComponent didn't track this frame
	if (!before.component.empty())
	{
		ComponentSnapshot after = takeSnapshot(entityFromId(before.entity), before.component);
		if (after.present != before.present || after.data != before.data)
		{
			const auto key = std::make_pair(before.entity, before.component);
			auto it = s_pendingComponents.find(key);
			if (it == s_pendingComponents.end())
				s_pendingComponents.emplace(key, PendingEdit<ComponentSnapshot>{ before, after, label });
			else
				it->second.after = after;
		}
	}

	recordFinishedEdits();
}

AssetSnapshot InspectorUndo::trackAsset(UUID asset)
{
	return takeAssetSnapshot(asset);
}

void InspectorUndo::commitAsset(const AssetSnapshot& before, const std::string& label)
{
	AssetSnapshot after = takeAssetSnapshot(before.asset);
	if (after.data != before.data)
	{
		const std::string key = before.asset.str();
		auto it = s_pendingAssets.find(key);
		if (it == s_pendingAssets.end())
			s_pendingAssets.emplace(key, PendingEdit<AssetSnapshot>{ before, after, label });
		else
			it->second.after = after;
	}

	recordFinishedEdits();
}
