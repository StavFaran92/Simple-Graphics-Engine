#include "CreateMenu.h"

#include <imgui.h>

#include "EditorState.h"
#include "EditorContext.h"

void CreateMenu::display()
{
	if (ImGui::BeginMenu("Create")) {
		if (ImGui::MenuItem("Folder")) {
			EditorContext::Instance().folderCreateDialog.activate();
			//EditorState::Instance().showMaterialCreateWindow = true;

		}
		if (ImGui::MenuItem("Material")) {
			EditorContext::Instance().materialCreateDialog.activate();
			//EditorState::Instance().showMaterialCreateWindow = true;

		}
		if (ImGui::MenuItem("Texture")) {
			EditorContext::Instance().textureCreateDialog.activate();
		}
		if (ImGui::MenuItem("Shader")) {
			EditorContext::Instance().shaderCreateDialog.activate();
		}

		if (ImGui::MenuItem("Lua Script")) {
			EditorContext::Instance().luaScriptCreateDialog.activate();
		}

		if (ImGui::MenuItem("Scene")) {
			EditorContext::Instance().sceneCreateDialog.activate();

		}
		ImGui::EndMenu();
	}
}
