#include "Dialogs.h"

#include "EditorState.h"
#include "imgui.h"
#include "NativeScriptsLoader.h"
#include "Common.h"
#include "Widgets.h"

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
			ResourceWrapper<Texture> displayTexture(textureList.at(selectedTextureIndex).uuid);
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
				EditorState::Instance().assetTextureSelectCB(textureList[selectedTextureIndex].uuid);

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

void displayTextureCreatorDialog()
{
	if (EditorState::Instance().showTextureCreateWindow)
	{
		ImGui::OpenPopup("CreateEmptyTexture");
		EditorState::Instance().showTextureCreateWindow = false;
	}
	if (ImGui::BeginPopupModal("CreateEmptyTexture", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		static int width = 512;
		static int height = 512;
		static char textureName[256] = "NewTexture";

		ImGui::InputInt("Width", &width);
		ImGui::InputInt("Height", &height);
		ImGui::InputText("Name", textureName, IM_ARRAYSIZE(textureName));

		ImGui::Separator();

		if (ImGui::Button("OK", ImVec2(120, 0)))
		{
			auto texture = Texture::createEmptyTexture(width, height);
			Texture::addTexture2D(textureName, texture);
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel", ImVec2(120, 0)))
		{
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}

void displayShaderCreatorDialog()
{
	if (EditorState::Instance().showShaderCreateWindow)
	{
		ImGui::OpenPopup("CreateShader");
		EditorState::Instance().showShaderCreateWindow = false;
	}
	if (ImGui::BeginPopupModal("CreateShader", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		static char shaderName[256] = "New Shader";
		static char filepath[256] = "";
		static int shaderOverrideType = 0;

		// Shader Name
		ImGui::Text("Name");
		ImGui::InputText("##ShaderName", shaderName, IM_ARRAYSIZE(shaderName), ImGuiInputTextFlags_EnterReturnsTrue);

		// Shader File Path
		ImGui::Text("Filepath");
		ImGui::InputText("##ShaderFilePath", filepath, IM_ARRAYSIZE(filepath), ImGuiInputTextFlags_EnterReturnsTrue);

		// Override Type Drop-down
		const char* overrideTypes[] = { "PBR Basic Shader", "Pixel Shader" };
		ImGui::Text("Override Type");
		ImGui::Combo("##ShaderOverrideType", (int*)&shaderOverrideType, overrideTypes, IM_ARRAYSIZE(overrideTypes));

		ImGui::Separator();

		if (ImGui::Button("OK", ImVec2(120, 0)))
		{
			auto& shader = Shader::createOverrideShader(shaderName, filepath, (ShaderOverride)shaderOverrideType);
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel", ImVec2(120, 0)))
		{
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}

void displayLuaScriptCreatorDialog()
{
	if (EditorState::Instance().showLuaScriptCreateWindow)
	{
		ImGui::OpenPopup("CreateLuaScript");
		EditorState::Instance().showLuaScriptCreateWindow = false;
	}
	if (ImGui::BeginPopupModal("CreateLuaScript", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		static char luaScriptName[256] = "NewLuaScript";

		ImGui::InputText("Name", luaScriptName, IM_ARRAYSIZE(luaScriptName));

		ImGui::Separator();

		if (ImGui::Button("OK", ImVec2(120, 0)))
		{
			ResourceWrapper<LuaScript> script = LuaScript::create();

			AssetCreateDescriptor desc;
			desc.aType = AssetType::LUA_SCRIPT;
			desc.name = luaScriptName;
			Engine::get()->getSubSystem<Assets>()->createAsset(script, desc);
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel", ImVec2(120, 0)))
		{
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}

void displayMaterialEditDialog()
{
	static ResourceWrapper<Material> previousMaterial;
	if (EditorState::Instance().showMaterialEditWindow)
	{
		ImGui::OpenPopup("EditMaterial");
		
		previousMaterial = EditorState::Instance().selectedMaterialForEdit.get()->clone(true);
		EditorState::Instance().showMaterialEditWindow = false;
	}
	if (ImGui::BeginPopupModal("EditMaterial", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		auto& mat = EditorState::Instance().selectedMaterialForEdit;
		ImGui::Text(mat->getName().c_str());

		ImGui::Dummy(ImVec2(0, 4));

		ImGui::ColorEdit3("Base Color", glm::value_ptr(mat->colorDiffuse));
		ImGui::DragFloat("Metallic", &mat->metallicFactor, 0.01f, 0.0f, 1.0f);
		ImGui::DragFloat("Roughness", &mat->roughnessFactor, 0.01f, 0.0f, 1.0f);
		ImGui::DragFloat("Opacity", &mat->opacityFactor, 0.01f, 0.0f, 1.0f);

		addSamplerEditWidget(mat, { 40, 40 }, "Albedo", Texture::TextureType::Albedo);
		addSamplerEditWidget(mat, { 40, 40 }, "Normal", Texture::TextureType::Normal);
		addSamplerEditWidget(mat, { 40, 40 }, "Metallic", Texture::TextureType::Metallic);
		addSamplerEditWidget(mat, { 40, 40 }, "Roughness", Texture::TextureType::Roughness);
		addSamplerEditWidget(mat, { 40, 40 }, "Ambient Occlusion", Texture::TextureType::AmbientOcclusion);

		ImGui::Separator();

		if (ImGui::Button("OK", ImVec2(120, 0)))
		{
			//Material::updateAsset(mat, {});
			Engine::get()->getSubSystem<Assets>()->updateAsset(mat);
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel", ImVec2(120, 0)))
		{
			mat = previousMaterial;
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}

void displayAssetSelectDialog(AssetType aType, UUID& uuid)
{
	if (EditorState::Instance().showAssetSelectorWindow)
	{
		std::string label = "Select " + getAssetTypeAsStr(aType);
		ImGui::Begin(label.c_str(), &EditorState::Instance().showAssetSelectorWindow, ImGuiWindowFlags_AlwaysAutoResize);
		ImGui::Text("Available Animations:");
		ImGui::Separator();

		static int selectedAssetIndex = -1;

		auto& assetList = Engine::get()->getSubSystem<Assets>()->getAllAssetsOfType(aType);

		for (int i = 0; i < assetList.size(); i++)
		{
			bool isSelected = (selectedAssetIndex == i);
			if (isSelected)
			{
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.2f, 0.2f, 1.0f)); // Change background color
			}
			if (!isSelected)
			{
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f)); // Default color
			}

			if (ImGui::Selectable(assetList[i].name.c_str()))
			{
				selectedAssetIndex = i;
			}

			ImGui::PopStyleColor();
		}

		ImGui::Separator();

		if (ImGui::Button("OK")) 
		{
			if (selectedAssetIndex >= 0 && selectedAssetIndex < assetList.size())
			{
				uuid = assetList[selectedAssetIndex].uuid;

			}
			EditorState::Instance().showAssetSelectorWindow = false;
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel")) 
		{
			EditorState::Instance().showAssetSelectorWindow = false;
		}

		ImGui::End();
	}
}
