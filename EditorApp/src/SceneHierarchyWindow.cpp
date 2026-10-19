#include "SceneHierarchyWindow.h"

#include <algorithm>
#include <vector>

#include "EditorState.h"
#include "memory/Assets.h"

#include "EntityState.h"
#include "EntityPresets.h"
#include <imgui_stdlib.h>

// Records a just-created entity as an undo step and selects it
static void recordCreated(Entity e, const std::string& label)
{
	CommandStack::Instance().record(std::make_unique<CreateEntitiesCmd>(e, label));
	updateScene();
	state.selectEntity(e);
}

// Draws the presets and submenus directly under 'prefix' ("" = top level, "Environment/" = inside Environment)
static void drawPresetMenu(const std::string& prefix)
{
	std::vector<std::string> drawn;
	for (const auto& preset : entityPresets())
	{
		if (preset.path.rfind(prefix, 0) != 0)
			continue;

		const std::string rest = preset.path.substr(prefix.size());
		const size_t slash = rest.find('/');
		const std::string label = rest.substr(0, slash);

		// Each submenu once, at its first preset
		if (std::find(drawn.begin(), drawn.end(), label) != drawn.end())
			continue;
		drawn.push_back(label);

		if (slash == std::string::npos)
		{
			if (ImGui::MenuItem(label.c_str()))
				recordCreated(preset.create(), "Create " + label);
		}
		else if (ImGui::BeginMenu(label.c_str()))
		{
			drawPresetMenu(prefix + label + "/");
			ImGui::EndMenu();
		}
	}
}

void displayEntityHelper(Entity& e)
{
	auto& transform = e.getComponent<Transformation>();
	auto& obj = e.getComponent<ObjectComponent>();



	if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
	{
		state.selectEntity(e);
	}

	if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
	{
		state.selectEntity(e);
		ImGui::OpenPopup("SceneObjectContextPopup");
	}



	if (state.getSelectedEntity() == e)
	{
		if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem))
		{
			if (ImGui::IsMouseDoubleClicked(0))
			{
				focusOnEntity(e, g_editorCamera);
			}
		}

		if (ImGui::BeginPopup("SceneObjectContextPopup"))
		{
			if (ImGui::MenuItem("Rename"))
			{
				// Focus the input text when renaming
				EditorState::Instance().selectedEntityRename = true;
			}

			if (ImGui::MenuItem("Create Prefab"))
			{
				std::string assetName = state.getSelectedEntity().getComponent<ObjectComponent>().name;

				AssetBuildDescriptor aInfo;
				aInfo.aType = AssetType::PREFAB;
				aInfo.name = assetName;
				PrefabCreateDescriptor createDesc;
				createDesc.data = Prefab::serializeEntityToPrefabData(e);
				// TODO: makeResourceCreateDescriptor<PrefabCreateDescriptor>() with entity
				Engine::get()->getSubSystem<Assets>()->createAsset(aInfo, createDesc);

			}

			if (e.HasComponent<CameraComponent>())
			{
				if (ImGui::MenuItem("Set as Primary Camera"))
				{
					auto scene = Engine::get()->getContext()->getActiveScene();
					scene->setGameCamera(e);
				}
			}

			if (ImGui::MenuItem("Remove from parent")) // todo fix this nonsense
			{
				reparentWithUndo(e, Entity::EmptyEntity, "Remove " + obj.name + " from parent");
			}

			if (ImGui::MenuItem("Delete"))
			{
				std::string error;
				if (!CommandStack::Instance().run(std::make_unique<DeleteEntitiesCmd>(e, "Delete " + obj.name), error))
					logError("Delete failed: " + error);
			}

			ImGui::EndPopup();

		}

		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
		{
			ImGui::SetDragDropPayload("DND_ITEM", &e, sizeof(Entity));
			ImGui::Text("Dragging %s", obj.name.c_str());
			ImGui::EndDragDropSource();
		}
	}

	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_ITEM"))
		{
			Entity sourceEntity = *(const Entity*)payload->Data;
			if (sourceEntity != e)
			{
				reparentWithUndo(sourceEntity, e, "Move " + sourceEntity.getComponent<ObjectComponent>().name + " under " + obj.name);
			}
		}
		ImGui::EndDragDropTarget();
	}

	ImGui::SameLine();
	ImGui::SetNextItemWidth(150.0f);
	if (EditorState::Instance().selectedEntityRename && state.getSelectedEntity() == e)
	{
		ImGui::SetKeyboardFocusHere();
		std::string& renameText = state.getCurrentEntityState().renameBuffer;

		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0)); // Fully transparent background
		ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0, 0, 0, 0));

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

		ImGui::InputText(("##edit_" + obj.name).c_str(), &renameText);

		ImGui::PopStyleVar();
		ImGui::PopStyleColor(3); // Pop all three

		if (ImGui::IsItemDeactivated())
		{
			if (renameText != obj.name)
			{
				const std::string component = componentTypeName<ObjectComponent>();
				ComponentSnapshot before = takeSnapshot(e, component);
				const std::string oldName = obj.name;
				obj.name = renameText;
				CommandStack::Instance().record(std::make_unique<EditComponentsCmd>(
					std::vector<ComponentSnapshot>{ before }, std::vector<ComponentSnapshot>{ takeSnapshot(e, component) },
					"Rename " + oldName + " to " + renameText));
			}
			updateScene(); // Assuming this updates any necessary scene state
			EditorState::Instance().selectedEntityRename = false;
		}
		//ImGui::InputText("##edit", &yourNameString);
	}
	else
	{
		ImGui::Text(obj.name.c_str());
	}
}

void displayEntity(Entity& e)
{
	auto& transform = e.getComponent<Transformation>();
	auto& obj = e.getComponent<ObjectComponent>();
	bool hasChildren = transform.getChildren().size() > 0;

	static int nonLeafTreeFlags = ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_OpenOnArrow;
	static int leafTreeFlags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_SpanAvailWidth;

	// Scope ImGui IDs by entity id - tree nodes are labeled by name, and names aren't unique (e.g. prefab clones)
	ImGui::PushID(static_cast<int>(e.handlerID()));

	ImGui::SetNextItemWidth(300.0f);


	if (hasChildren)
	{
		int flags = nonLeafTreeFlags;
		flags = (state.getSelectedEntity() == e) ? flags | ImGuiTreeNodeFlags_Selected : flags;
		bool isOpen = ImGui::TreeNodeEx(("##" + obj.name).c_str(), flags);

		displayEntityHelper(e);

		if (isOpen)
		{
			// m_children is an unordered_map - sort by entity id so children keep a stable display order
			std::vector<Entity> childrens;
			for (auto& [_, child] : transform.getChildren())
			{
				childrens.push_back(child);
			}
			std::sort(childrens.begin(), childrens.end(), [](const Entity& a, const Entity& b) {
				return a.handlerID() < b.handlerID();
			});

			for (auto& child : childrens)
			{
				displayEntity(child);
			}
			ImGui::TreePop();
		}
	}
	else
	{
		int flags = leafTreeFlags;
		flags = (state.getSelectedEntity() == e) ? flags | ImGuiTreeNodeFlags_Selected : flags;
		ImGui::TreeNodeEx(("##" + obj.name).c_str(), flags);
		displayEntityHelper(e);
	}

	ImGui::PopID();

}

void displaySceneObjects()
{
	// todo fix this shitty hack
	auto sceneName = "Scene " + std::to_string(Engine::get()->getContext()->getActiveSceneID());
	ImGui::Text(sceneName.c_str());

	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_ITEM"))
		{
			Entity sourceEntity = *(const Entity*)payload->Data;
			reparentWithUndo(sourceEntity, Entity::EmptyEntity, "Move " + sourceEntity.getComponent<ObjectComponent>().name + " to root");
		}
		ImGui::EndDragDropTarget();
	}

	ImGui::Separator();

	ImGui::BeginChild("##items", ImGui::GetContentRegionAvail(), true, ImGuiWindowFlags_NoScrollbar);

	// Registry iteration order changes as entities are added/removed - sort roots by entity id
	// so the hierarchy keeps a stable display order.
	std::vector<Entity> roots;
	for (auto& sceneObject : sceneObjects)
	{
		// if has parent it will be rendered in the recursive call (can be optimized if needed)
		if (sceneObject.e.getComponent<Transformation>().getParent().valid())
			continue;

		roots.push_back(sceneObject.e);
	}
	std::sort(roots.begin(), roots.end(), [](const Entity& a, const Entity& b) {
		return a.handlerID() < b.handlerID();
	});

	for (auto& root : roots)
	{
		displayEntity(root);
	}

	ImGui::EndChild(); // End background drop zone
}

void SceneHierarchyWindow::display()
{
	ImGui::Begin("Scene Hierarchy", nullptr, windowFlags);

	//displayWindowHeader("Scene Hierarchy");

	float windowWidth = ImGui::GetContentRegionAvail().x;
	if (ImGui::Button("+", ImVec2(windowWidth, 0)))
	{
		ImGui::OpenPopup("AddObjectToScenePopup");
	}

	if (ImGui::BeginPopup("AddObjectToScenePopup"))
	{
		drawPresetMenu("");
		ImGui::EndPopup();
	}

	ImVec2 listBoxSize = ImGui::GetContentRegionAvail();

	// Render list view
	if (ImGui::BeginListBox("##Objects", listBoxSize))
	{
		// Iterate through each scene object and render it as a selectable item in the list
		displaySceneObjects();
		ImGui::EndListBox();
	}

	ImGui::End();
}