#include "Dialogs.h"

#include "EditorState.h"
#include "imgui.h"
#include "NativeScriptsLoader.h"
#include "Common.h"

void displaySelectMeshDialog(std::string& uuid)
{
	if (EditorState::Instance().showMeshSelector)
	{
		ImGui::Begin("Select Mesh", &EditorState::Instance().showMeshSelector, ImGuiWindowFlags_AlwaysAutoResize);
		ImGui::Text("Available Meshes:");
		ImGui::Separator();

		static int selectedMeshIndex = -1;

		auto& meshList = Engine::get()->getSubSystem<Assets>()->getAllAssetsOfType(AssetType::MESH); // todo fix

		for (int i = 0; i < meshList.size(); i++)
		{
			bool isSelected = (selectedMeshIndex == i);
			if (isSelected)
			{
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.2f, 0.2f, 1.0f)); // Change background color
			}
			if (!isSelected)
			{
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f)); // Default color
			}

			if (ImGui::Selectable(meshList[i].name.c_str()))
			{
				selectedMeshIndex = i;
			}

			ImGui::PopStyleColor();
		}

		ImGui::Separator();

		if (ImGui::Button("OK")) {
			if (selectedMeshIndex >= 0 && selectedMeshIndex < meshList.size())
			{
				uuid = meshList[selectedMeshIndex].uuid;

			}
			EditorState::Instance().showMeshSelector = false;
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel")) {
			EditorState::Instance().showMeshSelector = false;
		}

		ImGui::End();
	}
}

void displaySelectAnimationDialog(std::string& uuid)
{
	if (EditorState::Instance().showAnimationSelector)
	{
		ImGui::Begin("Select Animation", &EditorState::Instance().showAnimationSelector, ImGuiWindowFlags_AlwaysAutoResize);
		ImGui::Text("Available Animations:");
		ImGui::Separator();

		static int selectedAnimationIndex = -1;

		auto& animationList = Engine::get()->getSubSystem<Assets>()->getAllAssetsOfType(AssetType::ANIMATION);

		for (int i = 0; i < animationList.size(); i++)
		{
			bool isSelected = (selectedAnimationIndex == i);
			if (isSelected)
			{
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.2f, 0.2f, 1.0f)); // Change background color
			}
			if (!isSelected)
			{
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f)); // Default color
			}

			if (ImGui::Selectable(animationList[i].name.c_str()))
			{
				selectedAnimationIndex = i;
			}

			ImGui::PopStyleColor();
		}

		ImGui::Separator();

		if (ImGui::Button("OK")) {
			if (selectedAnimationIndex >= 0 && selectedAnimationIndex < animationList.size())
			{
				uuid = animationList[selectedAnimationIndex].uuid;

			}
			EditorState::Instance().showAnimationSelector = false;
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel")) {
			EditorState::Instance().showAnimationSelector = false;
		}

		ImGui::End();
	}
}

void displaySelectScriptDialog(std::string& scriptName)
{
	if (EditorState::Instance().showScriptSelector)
	{
		ImGui::Begin("Select Script", &EditorState::Instance().showScriptSelector, ImGuiWindowFlags_AlwaysAutoResize);
		ImGui::Text("Available Scripts:");
		ImGui::Separator();

		static int selectedScriptIndex = -1;

		std::vector<std::string> scriptNamesList;
		NativeScriptsLoader::instance->getAllScripts(scriptNamesList);

		for (int i = 0; i < scriptNamesList.size(); i++)
		{
			bool isSelected = (selectedScriptIndex == i);

			if (ImGui::Selectable(scriptNamesList[i].c_str(), &isSelected))
			{
				selectedScriptIndex = i;
			}
		}

		ImGui::Separator();

		if (ImGui::Button("OK"))
		{
			if (selectedScriptIndex >= 0 && selectedScriptIndex < scriptNamesList.size())
			{
				scriptName = scriptNamesList[selectedScriptIndex];

			}
			EditorState::Instance().showScriptSelector = false;
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel"))
		{
			EditorState::Instance().showScriptSelector = false;
		}

		ImGui::End();
	}
}

void displaySelectShaderDialog(std::string& uuid)
{
	if (EditorState::Instance().showShaderSelector)
	{
		ImGui::Begin("Select Shader", &EditorState::Instance().showShaderSelector, ImGuiWindowFlags_AlwaysAutoResize);
		ImGui::Text("Available Shaders:");
		ImGui::Separator();

		static int selectedShaderIndex = -1;

		auto& shaderList = Engine::get()->getSubSystem<Assets>()->getAllAssetsOfType(AssetType::SHADER);


		for (int i = 0; i < shaderList.size(); i++)
		{
			bool isSelected = (selectedShaderIndex == i);

			if (ImGui::Selectable(shaderList[i].name.c_str(), &isSelected))
			{
				selectedShaderIndex = i;
			}
		}

		ImGui::Separator();

		if (ImGui::Button("OK"))
		{
			if (selectedShaderIndex >= 0 && selectedShaderIndex < shaderList.size())
			{
				uuid = shaderList[selectedShaderIndex].uuid;

			}
			EditorState::Instance().showShaderSelector = false;
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel"))
		{
			EditorState::Instance().showShaderSelector = false;
		}

		ImGui::End();
	}
}

void displayEntitySelectDialog(Entity& entity)
{
	if (ImGui::BeginPopup("EntitySelectPopup")) {



		ImGui::Text("Available Entities:");

		ImGui::Separator();

		static Entity selectedEntity = Entity::EmptyEntity;

		for (int i = 0; i < sceneObjects.size(); ++i)
		{
			auto& sceneObject = sceneObjects[i];
			auto& obj = sceneObject.e.getComponent<ObjectComponent>();

			bool isSelected = (selectedEntity == sceneObject.e);

			if (ImGui::Selectable(obj.name.c_str(), &isSelected))
			{
				selectedEntity = sceneObject.e;
			}
		}

		ImGui::Separator();

		if (ImGui::Button("OK")) 
		{
			if (selectedEntity != Entity::EmptyEntity)
			{
				entity = selectedEntity;

			}
			ImGui::CloseCurrentPopup();
			selectedEntity = Entity::EmptyEntity;
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel")) 
		{
			ImGui::CloseCurrentPopup();
			selectedEntity = Entity::EmptyEntity;
		}



		ImGui::EndPopup();
	}

}

void displayTextureSelectDialog()
{
	if (ImGui::BeginPopup("EditTexturePopup")) {

		auto assets = Engine::get()->getSubSystem<Assets>();

		ImGui::Text("Available Textures:");
		ImGui::Separator();

		static int selectedTextureIndex = -1;

		auto& textureList = assets->getAllAssetsOfType(AssetType::TEXTURE);

		if (selectedTextureIndex != -1)
		{
			Resource<Texture> displayTexture(textureList.at(selectedTextureIndex).uuid);
			ImVec2 imageSize(150, 150);
			ImGui::Image(reinterpret_cast<ImTextureID>(displayTexture.get()->getID()), imageSize, ImVec2(0, 1), ImVec2(1, 0), ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 1));
		}

		ImGui::Separator();

		for (int i = 0; i < textureList.size(); i++)
		{
			bool isSelected = (selectedTextureIndex == i);
			if (isSelected)
			{
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.2f, 0.2f, 1.0f)); // Change background color
			}
			if (!isSelected)
			{
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f)); // Default color
			}

			if (ImGui::Selectable(textureList[i].name.c_str(), false, ImGuiSelectableFlags_DontClosePopups))
			{
				selectedTextureIndex = i;
			}

			ImGui::PopStyleColor();
		}

		ImGui::Separator();

		if (ImGui::Button("OK")) {
			if (selectedTextureIndex >= 0 && selectedTextureIndex < textureList.size())
			{
				assetTextureSelectCB(textureList[selectedTextureIndex].uuid);

			}
			ImGui::CloseCurrentPopup();
			selectedTextureIndex = -1;
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel")) {
			ImGui::CloseCurrentPopup();
			selectedTextureIndex = -1;
		}



		ImGui::EndPopup();
	}

}