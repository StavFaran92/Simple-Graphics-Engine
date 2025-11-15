#pragma once

#include "sge.h"
#include "imgui.h"
#include "ImGuizmo.h"

#include <unordered_map>
#include <string>
#include <vector>

inline const ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoBringToFrontOnFocus |
											ImGuiWindowFlags_NoCollapse |
											ImGuiWindowFlags_NoFocusOnAppearing |
											ImGuiWindowFlags_NoTitleBar |
											ImGuiWindowFlags_NoScrollbar |
											ImGuiWindowFlags_NoScrollWithMouse |
											ImGuiWindowFlags_NoMove;

static const char* rigidyBodyTypesStrList[]{
	"Static",
	"Dynamic",
	"Kinematic"
};

static const char* renderTechniqueStrList[]{
	"Forward",
	"Defererred"
};

static const char* layerMaskList[]{
	"Layer Mask 0",
	"Layer Mask 1",
	"Layer Mask 2",
	"Layer Mask 3",
	"Layer Mask 4",
	"Layer Mask 5",
	"Layer Mask 6",
	"Layer Mask 7",
	"Layer Mask 8",
	"Layer Mask 9",
	"Layer Mask 10",
	"Layer Mask 11",
	"Layer Mask 12",
	"Layer Mask 13",
	"Layer Mask 14",
	"Layer Mask 15",
};

enum class LightType {
	DirectionalLight = 0,
	PointLight = 1
};

extern std::unordered_map<std::string, ResourceWrapper<Texture>> icons;

struct SceneObject 
{
	std::string name;
	Entity e;
};

extern std::vector<SceneObject> sceneObjects;

static void updateScene()
{
	sceneObjects.clear();
	for (auto&& [entity, obj] : Engine::get()->getContext()->getActiveScene()->getRegistry().get().view<ObjectComponent>().each())
	{
		sceneObjects.emplace_back(SceneObject{ obj.name, obj.e });
	}
}

template<typename T> 
static void displayComponent(const std::string& componentName, std::function<void(T&)> func)
{
	if (state.getSelectedEntity().HasComponent<T>())
	{
		ImVec2 startPos = ImGui::GetCursorScreenPos();
		ImVec2 startPosCursor = ImGui::GetCursorPos(); // Capture the initial cursor position

		displayColoredLabelWidget(componentName.c_str());
		auto& component = state.getSelectedEntity().getComponent<T>();

		ImVec2 cursorPos = ImGui::GetCursorPos();
		ImVec2 windowSize = ImGui::GetWindowSize();

		if (!std::is_same<T, Transformation>::value)
		{
			ImGui::SetCursorPos(ImVec2(windowSize.x - 24.0f, cursorPos.y - ImGui::GetTextLineHeightWithSpacing() - 8.0f));
			ImGui::PushID(componentName.c_str());
			ImVec2 size(10, 10); // size of the hitbox

			auto pos = ImGui::GetCursorPos();
			if (ImGui::InvisibleButton("##X", size))
			{
				state.getSelectedEntity().RemoveComponent<T>();
				ImGui::EndGroup();
				updateScene();
				return;
			}
			ImGui::SetCursorPos(pos);
			ImGui::Text("X"); // draw your own label or icon
			
			ImGui::PopID();
		}

		ImGui::Dummy(ImVec2(0, 5));

		ImGui::Indent(5); // Indent by 10 pixels
		func(component);
		ImGui::Unindent(5); // Remove the indent

		ImVec2 endPosCursor = ImGui::GetCursorPos(); // Capture the cursor position before adding the separator
		ImVec2 endPos = ImVec2(startPos.x + ImGui::GetContentRegionAvail().x, startPos.y + (endPosCursor.y - startPosCursor.y));

		ImGui::Dummy(ImVec2(0, 4));

		ImGui::Separator();

		ImGui::Dummy(ImVec2(0, 4));

		// Adjust the rectangle to the correct end position
		//ImGui::GetWindowDrawList()->AddRect(startPos, endPos, ImGui::GetColorU32(ImGuiCol_Header), 0.f, 0, 2.f);
	}
}