#pragma once

#include "sge.h"
#include "imgui.h"

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

extern std::unordered_map<std::string, Resource<Texture>> icons;

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

class EntityStates
{
public:
	void selectEntity(Entity e)
	{
		m_selectedEntity = e;
		Engine::get()->getSubSystem<ObjectPicker>()->setSelectedObject(m_selectedEntity.handlerID());

		if (e == Entity::EmptyEntity)
		{
			return;
		}

		std::shared_ptr<EntityState> eState = std::make_shared<EntityState>(e);
		eState->update();

		m_states[e.handlerID()] = eState;
	}

	EntityState& getCurrentEntityState()
	{
		auto iter = m_states.find(m_selectedEntity.handlerID());
		if (iter == m_states.end())
		{
			auto eState = std::make_shared<EntityState>(m_selectedEntity);
			eState->update();
			m_states[m_selectedEntity.handlerID()] = eState;
			return *eState.get();
		}

		return *iter->second.get();
	}

	Entity getSelectedEntity() const
	{
		return m_selectedEntity;
	}

private:
	std::unordered_map<entity_id, std::shared_ptr<EntityState>> m_states;
	Entity m_selectedEntity = Entity::EmptyEntity;
};

extern EntityStates state;

template<typename T> 
static void displayComponent(const std::string& componentName, std::function<void(T&)> func)
{
	if (state.getSelectedEntity().HasComponent<T>())
	{
		ImVec2 startPos = ImGui::GetCursorScreenPos();
		ImVec2 startPosCursor = ImGui::GetCursorPos(); // Capture the initial cursor position

		AddColoredLabel(componentName.c_str());
		auto& component = state.getSelectedEntity().getComponent<T>();

		ImVec2 cursorPos = ImGui::GetCursorPos();
		ImVec2 windowSize = ImGui::GetWindowSize();

		if (!std::is_same<T, Transformation>::value)
		{
			ImGui::SetCursorPos(ImVec2(windowSize.x - 24.0f, cursorPos.y - ImGui::GetTextLineHeightWithSpacing() - 7.0f));
			ImGui::PushID(componentName.c_str());
			if (ImGui::Button("X")) {
				state.getSelectedEntity().RemoveComponent<T>();
				ImGui::EndGroup();
				updateScene();
				return;
			}
			ImGui::PopID();
		}

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