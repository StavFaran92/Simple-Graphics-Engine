#pragma once

#include "sge.h"
#include "imgui.h"
#include "imgui_internal.h"
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
		sceneObjects.emplace_back(SceneObject{ 
			obj.name, 
			Entity{entity, &Engine::get()->getContext()->getActiveScene()->getRegistry() } });
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

	// Sharper corners for component headers (global FrameRounding is too round here)
	const float componentRounding = 1.0f;
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, componentRounding);
	bool open = ImGui::CollapsingHeader(componentName.c_str(),
		ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap);
	ImGui::PopStyleVar();
	ImGui::SetItemAllowOverlap();

	// Header rect - the content box matches its width (framed headers extend past the content region)
	ImVec2 headerMin = ImGui::GetItemRectMin();
	ImVec2 headerMax = ImGui::GetItemRectMax();

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
		const ImGuiStyle& style = ImGui::GetStyle();

		// Horizontal padding on both sides of the component content
		const float contentPadding = 8.0f;

		ImGuiWindow* window = ImGui::GetCurrentWindow();
		const float prevWorkMaxX = window->WorkRect.Max.x;
		const float prevContentMaxX = window->ContentRegionRect.Max.x;
		window->WorkRect.Max.x -= contentPadding;
		window->ContentRegionRect.Max.x -= contentPadding;

		ImGui::Dummy(ImVec2(0, 4));
		ImGui::Indent(contentPadding);
		func(component);
		ImGui::Unindent(contentPadding);
		ImGui::Dummy(ImVec2(0, 4));

		window->WorkRect.Max.x = prevWorkMaxX;
		window->ContentRegionRect.Max.x = prevContentMaxX;

		// Wrap the component content in a rectangle, starting under the header and matching its width
		float bottom = ImGui::GetCursorScreenPos().y - style.ItemSpacing.y;

		ImGui::GetWindowDrawList()->AddRect(
			ImVec2(headerMin.x, headerMax.y),
			ImVec2(headerMax.x, bottom),
			ImGui::GetColorU32(ImVec4(0.35f, 0.39f, 0.45f, 1.0f)), // muted grey-blue, visible on the dark window bg
			componentRounding,
			ImDrawFlags_RoundCornersBottom,
			1.0f);

		// Gap between component boxes
		ImGui::Dummy(ImVec2(0, 2));
	}

	ImGui::PopID();
}

void focusOnEntity(Entity e, Entity cameraEntity);