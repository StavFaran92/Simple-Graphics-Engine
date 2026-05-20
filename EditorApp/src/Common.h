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

static const char* physicsCollisionTypesStrList[]{
	"Collider",
	"Trigger",
	"QueryOnly"
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

extern std::unordered_map<std::string, TextureResourceRef> icons;

struct SceneObject 
{
	std::string name;
	Entity e;
};

extern std::vector<SceneObject> sceneObjects;

extern Entity g_primaryCamera;
extern Entity g_editorCamera;

extern uint32_t g_previewWindowID;

static void updateScene()
{
	sceneObjects.clear();
	for (auto&& [entity, obj] : Engine::get()->getContext()->getActiveScene()->getRegistry().get().view<ObjectComponent>().each())
	{
		sceneObjects.emplace_back(SceneObject{ obj.name, obj.e });
	}
}

static void setupScene()
{
	auto scene = Engine::get()->getContext()->getActiveScene();

	if (scene.isEmpty())
	{
		logError("Empty scene detected.");
		return;
	}
	scene->addRenderView("Editor View", 0, 0, Engine::get()->getWindow()->getWidth(), Engine::get()->getWindow()->getHeight(), g_editorCamera);
	g_previewWindowID = scene->getGameRenderViewFrameBufferID();
	g_primaryCamera = scene->getGameCamera();
}

template<typename T>
static void displayComponent(const std::string& componentName, std::function<void(T&)> func)
{
	if (!state.getSelectedEntity().HasComponent<T>())
		return;

	auto& component = state.getSelectedEntity().getComponent<T>();

	ImGui::PushID(componentName.c_str());

	bool open = ImGui::CollapsingHeader(componentName.c_str(),
		ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap);
	ImGui::SetItemAllowOverlap();

	ImVec2 afterHeaderCursor = ImGui::GetCursorPos();
	ImVec2 windowSize = ImGui::GetWindowSize();
	float headerY = afterHeaderCursor.y - ImGui::GetFrameHeight() - ImGui::GetStyle().ItemSpacing.y;
	float rightX = windowSize.x - 8.0f;

	if (!std::is_same<T, Transformation>::value)
	{
		rightX -= 18.0f;
		ImGui::SetCursorPos(ImVec2(rightX, headerY + 2.0f));
		if (ImGui::SmallButton("X"))
		{
			ImGui::SetCursorPos(afterHeaderCursor);
			ImGui::PopID();
			state.getSelectedEntity().RemoveComponent<T>();
			updateScene();
			return;
		}

		rightX -= 26.0f;
		ImGui::SetCursorPos(ImVec2(rightX, headerY));
		ImGui::Checkbox("##isActive", &component.isActive);

		ImGui::SetCursorPos(afterHeaderCursor);
	}

	if (open)
	{
		ImGui::Dummy(ImVec2(0, 4));
		ImGui::Indent(5);
		func(component);
		ImGui::Unindent(5);
		ImGui::Dummy(ImVec2(0, 4));
	}

	ImGui::PopID();
}

void focusOnEntity(Entity e, Entity cameraEntity);