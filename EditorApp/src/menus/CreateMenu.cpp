#include "CreateMenu.h"

#include <imgui.h>

#include "EditorState.h"
#include "DialogManager.h"

void CreateMenu::display()
{
	if (ImGui::BeginMenu("Create")) {
		if (ImGui::MenuItem("Folder")) {
			DialogManager::Instance().folderCreateDialog.activate();
			//EditorState::Instance().showMaterialCreateWindow = true;

		}
		if (ImGui::MenuItem("Material")) {
			DialogManager::Instance().materialCreateDialog.activate();
			//EditorState::Instance().showMaterialCreateWindow = true;

		}
		if (ImGui::MenuItem("Texture")) {
			DialogManager::Instance().textureCreateDialog.activate();
		}
		if (ImGui::MenuItem("Shader")) {
			DialogManager::Instance().shaderCreateDialog.activate();
		}

		if (ImGui::MenuItem("Lua Script")) {
			DialogManager::Instance().luaScriptCreateDialog.activate();
		}

		if (ImGui::MenuItem("Scene")) {
			DialogManager::Instance().sceneCreateDialog.activate();

		}
		ImGui::EndMenu();
	}
}
