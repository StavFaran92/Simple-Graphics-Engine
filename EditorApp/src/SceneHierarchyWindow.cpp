#include "SceneHierarchyWindow.h"

#include "Dialogs.h"
#include "EditorState.h"
#include "memory/Assets.h"

#include "EntityState.h"
#include <imgui_stdlib.h>

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
				//focusOnEntity(e, g_ed); //TODO fix
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

				AssetCreateDescriptor aInfo;
				aInfo.aType = AssetType::PREFAB;
				aInfo.name = assetName;
				// TODO: makeResourceCreateDescriptor<PrefabCreateDescriptor>() with entity
				Engine::get()->getSubSystem<Assets>()->createAsset(aInfo);

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
				e.getComponent<Transformation>().removeParent();
			}

			if (ImGui::MenuItem("Delete"))
			{
				e.remove();
				updateScene();
				state.selectEntity(Entity::EmptyEntity);
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
				sourceEntity.getComponent<Transformation>().setParent(e);
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
			obj.name = renameText;
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

	ImGui::SetNextItemWidth(300.0f);


	if (hasChildren)
	{
		int flags = nonLeafTreeFlags;
		flags = (state.getSelectedEntity() == e) ? flags | ImGuiTreeNodeFlags_Selected : flags;
		bool isOpen = ImGui::TreeNodeEx(("##" + obj.name).c_str(), flags);

		displayEntityHelper(e);

		if (isOpen)
		{
			//ImGui::TreePush(obj.name.c_str());
			auto childrens = transform.getChildren();
			auto childIter = childrens.begin();
			while (childIter != childrens.end())
			{
				displayEntity(childIter->second);
				childIter++;
			}
			//ImGui::TreePop();
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
			sourceEntity.getComponent<Transformation>().removeParent();
		}
		ImGui::EndDragDropTarget();
	}

	ImGui::Separator();

	ImGui::BeginChild("##items", ImGui::GetContentRegionAvail(), true, ImGuiWindowFlags_NoScrollbar);

	for (int i = 0; i < sceneObjects.size(); ++i)
	{
		auto& sceneObject = sceneObjects[i];
		auto& transform = sceneObject.e.getComponent<Transformation>();

		// if has parent it will be rendered in the recursive call (can be optimized if needed)
		if (transform.getParent().valid())
			continue;

		ImGui::PushID(i); // Push a unique ID to avoid ImGui ID conflicts

		displayEntity(sceneObject.e);

		ImGui::PopID();
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
		if (ImGui::MenuItem("Entity"))
		{
			Engine::get()->getContext()->getActiveScene()->createEntity();
			updateScene();
			state.selectEntity(sceneObjects[0].e);
		}
		if (ImGui::BeginMenu("Primitive"))
		{ // Begin the submenu
			if (ImGui::MenuItem("Cube"))
			{
				ShapeFactory::createBoxEntity(&Engine::get()->getContext()->getActiveScene()->getRegistry());
				updateScene();
				state.selectEntity(sceneObjects[0].e);
			}
			if (ImGui::MenuItem("Sphere"))
			{
				ShapeFactory::createSphere(&Engine::get()->getContext()->getActiveScene()->getRegistry());
				updateScene();
				state.selectEntity(sceneObjects[0].e);
			}
			if (ImGui::MenuItem("Quad"))
			{
				ShapeFactory::createQuad(&Engine::get()->getContext()->getActiveScene()->getRegistry());
				updateScene();
				state.selectEntity(sceneObjects[0].e);
			}

			ImGui::EndMenu(); // End the submenu
		}

		if (ImGui::BeginMenu("Light"))
		{ // Begin the submenu
			if (ImGui::MenuItem("Directional Light"))
			{
				static int createdDLightCount = 0;
				auto e = Engine::get()->getContext()->getActiveScene()->createEntity("Directional_Light_" + std::to_string(createdDLightCount++));
				e.addComponent<DirectionalLight>(glm::vec3{ 0,0,0 }, glm::vec3{ 0,-1,0 }, 1.f, 1.f);
				updateScene();
				state.selectEntity(sceneObjects[0].e);
			}
			if (ImGui::MenuItem("Point Light"))
			{
				static int createdPLightCount = 0;
				auto e = Engine::get()->getContext()->getActiveScene()->createEntity("Point_Light_" + std::to_string(createdPLightCount++));
				e.addComponent<PointLight>(glm::vec3{ 0,0,0 }, 1.f, 1.f, Attenuation());
				updateScene();
				state.selectEntity(sceneObjects[0].e);
			}

			ImGui::EndMenu(); // End the submenu
		}

		if (ImGui::BeginMenu("Environment"))
		{ // Begin the submenu
			if (ImGui::MenuItem("Skybox"))
			{
				Entity e = Skybox::createSkybox(SGE_ROOT_DIR "Resources/Engine/Textures/sunflowers_puresky_4k.hdr", Skybox::TexType::EQUIRECTANGULAR);
				updateScene();
				state.selectEntity(sceneObjects[0].e);
			}

			if (ImGui::MenuItem("Terrain"))
			{
				Entity e = Terrain::createTerrain(100, 100);
				updateScene();
				state.selectEntity(sceneObjects[0].e);
			}

			if (ImGui::MenuItem("Pool"))
			{
				Entity e = WaterSystem::createPool();
				updateScene();
				state.selectEntity(sceneObjects[0].e);
			}

			if (ImGui::MenuItem("Clouds"))
			{
				Entity e = VolumetricCloudsSystem::createVolumetricClouds();
				updateScene();
				state.selectEntity(sceneObjects[0].e);
			}

			ImGui::EndMenu(); // End the submenu
		}

		// Add more submenus or menu items as needed
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