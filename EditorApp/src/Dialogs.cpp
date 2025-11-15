#include "Dialogs.h"

#include "EditorState.h"
#include "imgui.h"
#include "NativeScriptsLoader.h"
#include "Common.h"
#include "Widgets.h"
#include "tinyfiledialogs.h"

#include <imgui_stdlib.h>

#include "Widgets/UniqueNameWidget.h"
#include "Widgets/FilepathWidget.h"
#include "Widgets/TextureDataWidget.h"


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

void displaySelectShaderDialog(UUID& uuid)
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
			ResourceWrapper<Texture> displayTexture = textureList.at(selectedTextureIndex).resource.as<Texture>();
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
	static UniqueNameWidget uniqueName("Name");
	static TextureDataWidget textureDataWidget;
	if (EditorState::Instance().showTextureCreateWindow)
	{
		ImGui::OpenPopup("CreateEmptyTexture");
		EditorState::Instance().showTextureCreateWindow = false;

		uniqueName.name = Engine::get()->getSubSystem<UniqueNameManager>()->suggestUniqueName("NewTexture");
	}
	if (ImGui::BeginPopupModal("CreateEmptyTexture", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		static int width = 512;
		static int height = 512;
	
		uniqueName.draw();
		ImGui::InputInt("Width", &width);
		ImGui::InputInt("Height", &height);
		textureDataWidget.draw();

		ImGui::Separator();

		if (ImGui::Button("OK", ImVec2(120, 0)))
		{
			if (uniqueName.isValid())
			{
				auto texture = Texture::createEmptyTexture(width, height);
				
				AssetCreateDescriptor aInfo;
				aInfo.aType = AssetType::TEXTURE;
				aInfo.name = uniqueName.name;
				aInfo.attributes = texture->getTextureAssetAttributes().toMap();
				Engine::get()->getSubSystem<Assets>()->createAsset(texture, aInfo);

				ImGui::CloseCurrentPopup();
			}
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
	static UniqueNameWidget uniqueName("Name");
	static FilepathWidget filepath("##Filepath", Constants::g_shaderSupportedFormats, 1);
	if (EditorState::Instance().showShaderCreateWindow)
	{
		ImGui::OpenPopup("CreateShader");
		EditorState::Instance().showShaderCreateWindow = false;
		uniqueName.name = Engine::get()->getSubSystem<UniqueNameManager>()->suggestUniqueName("New Shader");
	}
	if (ImGui::BeginPopupModal("CreateShader", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		uniqueName.draw();
		filepath.draw();

		static ShaderOverride shaderOverrideType = ShaderOverride::None;

		// Override Type Drop-down
		int currentIndex = 0;
		int index = 0;
		std::vector<const char*> comboItems;

		for (const auto& [key, value] : shaderOverrideToString)
		{
			if (key == shaderOverrideType)
				currentIndex = index;

			comboItems.push_back(value.c_str());
			++index;
		}

		//const char* overrideTypes[] = { "PBR Basic Shader", "Pixel Shader", "Volume Shader", "Post Process Effect Shader"};
		ImGui::Text("Override Type");
		if(ImGui::Combo("##ShaderOverrideType", &currentIndex, comboItems.data(), static_cast<int>(comboItems.size())))
		{
			// Update the enum based on the selected index
			auto it = shaderOverrideToString.begin();
			std::advance(it, currentIndex);
			shaderOverrideType = it->first;
		}

		ImGui::Separator();

		if (ImGui::Button("OK", ImVec2(120, 0)))
		{
			auto& shader = Shader::createOverrideShader(filepath.m_filepath, shaderOverrideType); // todo fix

			AssetCreateDescriptor desc;
			desc.name = uniqueName.name;
			desc.origFilePath = filepath.m_filepath;
			desc.aType = AssetType::SHADER;
			desc.attributes[Shader::ATTRIB_SHADER_OVERRIDE] = Shader::getShaderOverrideAsStr(shaderOverrideType);
			Engine::get()->getSubSystem<Assets>()->createAsset(shader, desc);
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
	static UniqueNameWidget uniqueName("Name");
	if (EditorState::Instance().showLuaScriptCreateWindow)
	{
		ImGui::OpenPopup("CreateLuaScript");
		EditorState::Instance().showLuaScriptCreateWindow = false;
		uniqueName.name = Engine::get()->getSubSystem<UniqueNameManager>()->suggestUniqueName("NewLuaScript");
	}
	if (ImGui::BeginPopupModal("CreateLuaScript", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		uniqueName.draw();
		ImGui::Separator();

		if (ImGui::Button("OK", ImVec2(120, 0)))
		{
			if (uniqueName.isValid())
			{
				ResourceWrapper<LuaScript> script = LuaScript::create();

				AssetCreateDescriptor desc;
				desc.aType = AssetType::LUA_SCRIPT;
				desc.name = uniqueName.name;
				Engine::get()->getSubSystem<Assets>()->createAsset(script, desc);
				ImGui::CloseCurrentPopup();
			}

		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel", ImVec2(120, 0)))
		{
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}

void displayMaterialCreatorDialog()
{
	//static MaterialCreateDialog dialog;
	//dialog.appear();
	//dialog.draw(); //Love Love Love.... <3<3

	//static UniqueNameWidget uniqueName("Name");
	//static ResourceWrapper<Material> tempMaterial;
	//static MaterialDataWidget matData;
	//if (EditorState::Instance().showMaterialCreateWindow)
	//{
	//	ImGui::OpenPopup("Create Material");
	//	EditorState::Instance().showMaterialCreateWindow = false;
	//	tempMaterial = Material::create();
	//	uniqueName.name = Engine::get()->getSubSystem<UniqueNameManager>()->suggestUniqueName("New Material");
	//}
	//if (ImGui::BeginPopupModal("Create Material", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	//{
	//	uniqueName.draw();
	//	ImGui::Separator();
	//	matData.draw(tempMaterial);
	//	ImGui::Separator();

	//	if (ImGui::Button("OK", ImVec2(120, 0)))
	//	{
	//		if (uniqueName.isValid())
	//		{
	//			AssetCreateDescriptor desc;
	//			desc.aType = AssetType::MATERIAL;
	//			desc.name = uniqueName.name;
	//			Engine::get()->getSubSystem<Assets>()->createAsset(tempMaterial, desc);
	//			ImGui::CloseCurrentPopup();
	//		}

	//	}

	//	ImGui::SameLine();

	//	if (ImGui::Button("Cancel", ImVec2(120, 0)))
	//	{
	//		ImGui::CloseCurrentPopup();
	//	}

	//	ImGui::EndPopup();
	//}
}

void displayProjectSettingsDialog()
{
	if (EditorState::Instance().showSettingsWindow)
	{
		ImGui::OpenPopup("DisplayProjectSettingsDialog");
		EditorState::Instance().showSettingsWindow = false;
	}

	// Set constraints BEFORE BeginPopupModal
	ImGui::SetNextWindowSizeConstraints(ImVec2(300, 250), ImVec2(FLT_MAX, FLT_MAX));

	if (ImGui::BeginPopupModal("DisplayProjectSettingsDialog", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		auto graphics = Engine::get()->getSubSystem<Graphics>();

		ImGui::Checkbox("SSAO", &graphics->useSSAO);

		if (ImGui::Button("OK"))
		{
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}

void displayMaterialEditDialog()
{
	static ResourceWrapper<Material> previousMaterial;
	static MaterialDataWidget materialData;
	if (EditorState::Instance().showMaterialEditWindow)
	{
		ImGui::OpenPopup("Edit Material");
		
		previousMaterial = EditorState::Instance().selectedMaterialForEdit.get()->clone(true);
		EditorState::Instance().showMaterialEditWindow = false;
	}
	if (ImGui::BeginPopupModal("Edit Material", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		auto& mat = EditorState::Instance().selectedMaterialForEdit;

		materialData.draw(mat.resource());
		//ImGui::Text(mat.resource()->getName().c_str());

		//ImGui::Dummy(ImVec2(0, 4));



		//ImGui::ColorEdit3("Base Color", glm::value_ptr(mat.resource()->colorDiffuse));
		//ImGui::DragFloat("Metallic", &mat.resource()->metallicFactor, 0.01f, 0.0f, 1.0f);
		//ImGui::DragFloat("Roughness", &mat.resource()->roughnessFactor, 0.01f, 0.0f, 1.0f);
		//ImGui::DragFloat("Opacity", &mat.resource()->opacityFactor, 0.01f, 0.0f, 1.0f);

		//addSamplerEditWidget(mat, { 40, 40 }, "Albedo", Texture::TextureType::Albedo);
		//addSamplerEditWidget(mat, { 40, 40 }, "Normal", Texture::TextureType::Normal);
		//addSamplerEditWidget(mat, { 40, 40 }, "Metallic", Texture::TextureType::Metallic);
		//addSamplerEditWidget(mat, { 40, 40 }, "Roughness", Texture::TextureType::Roughness);
		//addSamplerEditWidget(mat, { 40, 40 }, "Ambient Occlusion", Texture::TextureType::AmbientOcclusion);

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
			mat.resource() = previousMaterial;
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