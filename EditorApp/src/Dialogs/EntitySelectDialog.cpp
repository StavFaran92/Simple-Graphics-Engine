#include "EntitySelectDialog.h"

#include "Common.h"
#include "EditorState.h"
#include "imgui.h"

EntitySelectDialog::EntitySelectDialog()
	: DialogBase("EntitySelectDialog")
{
}

void EntitySelectDialog::appearContent()
{
	m_selectedEntity = Entity::EmptyEntity;
}

void EntitySelectDialog::drawContent()
{
	ImGui::Text("Available Entities:");
	ImGui::Separator();

	for (int i = 0; i < static_cast<int>(sceneObjects.size()); ++i)
	{
		ImGui::PushID(i);
		auto& sceneObject = sceneObjects[i];
		auto& obj = sceneObject.e.getComponent<ObjectComponent>();
		bool isSelected = (m_selectedEntity == sceneObject.e);
		std::string name = obj.name + " (" + std::to_string(sceneObject.e.handlerID()) + ")";
		if (ImGui::Selectable(name.c_str(), &isSelected))
		{
			m_selectedEntity = sceneObject.e;
		}
		ImGui::PopID();
	}
}

bool EntitySelectDialog::acceptContent()
{
	if (m_selectedEntity != Entity::EmptyEntity && EditorState::Instance().entitySelectCB)
	{
		EditorState::Instance().entitySelectCB(m_selectedEntity);
	}
	return true;
}

void EntitySelectDialog::cancelContent()
{
	m_selectedEntity = Entity::EmptyEntity;
}
