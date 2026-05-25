#include "Widgets.h"

#include "EditorState.h"
#include <imgui_stdlib.h>
#include "tinyfiledialogs.h"
#include "dialogs/AssetSelectDialog.h"
#include "DialogManager.h"
#include "render/MaterialData.h"
#include "render/TerrainLayer.h"

void addTextureEditWidget(const std::string& name, TextureAssetRef texture, ImVec2 size, std::function<void(UUID uuid)> callback)
{
	ImGui::PushID(name.c_str());

	int texID = 0;
	if (!texture.isEmpty())
	{
		texID = texture.resource()->getID();
	}

	if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(texID), size))
	{
		DialogManager::Instance().textureSelectDialog.onAcceptCB = callback;
		DialogManager::Instance().textureSelectDialog.activate();
	}

	ImGui::PopID();
}

void addSamplerEditWidget(std::shared_ptr<TextureSamplerAsset> sampler, ImVec2 size, const std::string& name, const std::function<void(UUID)>& onAccpetCB)
{
	ImGui::PushID(name.c_str());

	int texID = 0;
	if (!sampler->texture.isEmpty())
	{
		texID = sampler->texture.resource()->getID();
	}

	if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(texID), size))
	{
		DialogManager::Instance().editSamplerDialog.sampler = sampler;
		DialogManager::Instance().editSamplerDialog.onAcceptCB = onAccpetCB;
		DialogManager::Instance().editSamplerDialog.activate();
	}

	ImGui::SameLine();
	ImGui::Text(name.c_str());

	ImGui::PopID();

}

void displayChannelSelectWidget(int*& currentChannel)
{
	static const char* channelMaskOptions[] = { "None", "R", "G", "B", "A" };
	if (ImGui::BeginCombo("##channelMask", channelMaskOptions[*currentChannel])) // Label for the combo box
	{
		for (int i = 0; i < IM_ARRAYSIZE(channelMaskOptions); i++)
		{
			bool isSelected = (*currentChannel == i);
			if (ImGui::Selectable(channelMaskOptions[i], isSelected))
			{
				*currentChannel = i; // Update selected index
			}

			if (isSelected)
				ImGui::SetItemDefaultFocus(); // Set focus to the current item
		}
		ImGui::EndCombo();
	}
}

void displayTextureWidget()
{
	if (EditorState::Instance().showTextureDisplayWindow)
	{
		ImGui::OpenPopup("Texture Preview");
		EditorState::Instance().showTextureDisplayWindow = false;
	}
	if (ImGui::BeginPopupModal("Texture Preview", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		const auto& selectedRes = DebugHelper::getInstance().getDebugTextures().at(EditorState::Instance().selectedTextureName);
		Texture* tex = selectedRes.get();

		if (tex)
		{
			ImTextureID texID = reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(tex->getID()));
			ImVec2 texSize(512, 512); // Preview size (can be dynamic)

			ImGui::Text("%s", EditorState::Instance().selectedTextureName.c_str());
			ImGui::Image(texID, texSize);
		}

		if (ImGui::Button("Close"))
		{
			ImGui::CloseCurrentPopup();
			EditorState::Instance().showTextureDisplayWindow = false;
		}

		ImGui::EndPopup();
	}
}

void displayColoredLabelWidget(const char* label)
{
	// Draw a blue background using ImGuiCol_Header color
	ImVec2 startPos = ImGui::GetCursorScreenPos();
	ImVec2 endPos = ImVec2(startPos.x + ImGui::GetContentRegionAvail().x, startPos.y + ImGui::GetTextLineHeightWithSpacing());
	ImGui::GetWindowDrawList()->AddRectFilled(startPos, endPos, ImGui::GetColorU32(ImGuiCol_Header));

	// Calculate the vertical offset to center the text within the rectangle
	float offsetY = (ImGui::GetTextLineHeightWithSpacing() - ImGui::GetFrameHeight()) * 0.5f;

	// Calculate padding values
	float paddingX = 5.0f;
	float paddingY = 3.0f;

	// Adjust the text position to center it vertically and add padding
	ImVec2 textPos = ImVec2(startPos.x + paddingX, startPos.y + offsetY + paddingY);

	// Render the label text
	ImGui::SetCursorScreenPos(textPos);
	ImGui::TextUnformatted(label);

	ImGui::Dummy(ImVec2(0.0f, 2.0f)); // Add a vertical gap
}

void addEntitySelectWidget(const std::string& label, Entity current, const std::function<void(Entity)>& onAcceptCB)
{
	std::string buttonLabel = current.valid() ? current.getComponent<ObjectComponent>().name : "None";

	ImGui::Text(label.c_str());
	ImGui::SameLine();

	ImGui::PushID(label.c_str());
	float width = ImGui::GetContentRegionAvail().x;
	if (ImGui::Button(buttonLabel.c_str(), ImVec2(width, 0)))
	{
		EditorState::Instance().entitySelectCB = onAcceptCB;
		DialogManager::Instance().entitySelectDialog.activate();
	}
	ImGui::PopID();
}

bool addAssetSelectWidget(const std::string& name, AssetType aType, const std::function<void(UUID)>& onAccpetCB)
{
	float width = ImGui::GetContentRegionAvail().x;

	if (ImGui::Button(name.c_str(), ImVec2(width, 0)))
	{
		DialogManager::Instance().assetSelectDialog.assetType = aType;
		DialogManager::Instance().assetSelectDialog.onAccpetCB = onAccpetCB;
		DialogManager::Instance().assetSelectDialog.activate();
	}

	return false;
}




void MaterialDataWidget::draw(MaterialData& data, const std::function<void(const MaterialData&)>& onChangedCB)
{
	ImGui::Text(data.name.c_str());

	ImGui::Dummy(ImVec2(0, 4));

	// TODO replace using magic enum
	static const char* RenderModeNames[] = {
		"Opaque",
		"Transparent",
		"Terrain",
		"Skybox",
		"Unlit",
		"UI",
		"Volume",
		"Custom"
	};

	static MaterialRenderMode currentMode;
	currentMode = data.getMaterialRenderMode();
	int currentIndex = static_cast<int>(currentMode);

	if (ImGui::BeginCombo("Render Mode", RenderModeNames[currentIndex])) {
		for (int i = 0; i < IM_ARRAYSIZE(RenderModeNames); ++i) {
			bool isSelected = (i == currentIndex);
			if (ImGui::Selectable(RenderModeNames[i], isSelected)) {
				currentIndex = i;
				currentMode = static_cast<MaterialRenderMode>(i);
				data.setMaterialRenderMode(currentMode);
				if(onChangedCB) 
					onChangedCB(data);
			}
			if (isSelected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	if (currentMode == MaterialRenderMode::Custom)
	{
		std::string shaderName = "None";
		if (!data.getCustomShader().isEmpty())
		{
			shaderName = data.getCustomShader().info().name;
		}

		addAssetSelectWidget(shaderName, AssetType::SHADER, [&data, onChangedCB](UUID uuid) {
			data.setCustomShader(ShaderAssetRef(uuid));
			if (onChangedCB) 
				onChangedCB(data);
		});
	}

	// Custom Textures Array
	auto& samplers = data.getAllProptiesOfType(MaterialPropertyType::SAMPLER);
	if (samplers.size() > 0 && ImGui::CollapsingHeader("Samplers"))
	{
		for (auto& [name, value] : samplers)
		{
			auto sampler = std::get<std::shared_ptr<TextureSamplerAsset>>(value);

			ImGui::PushID(name.c_str());
			ImGui::Text(name.c_str());
			if (ImGui::Checkbox("", (bool*)&sampler->state.isActive)) 
			{ 
				if (onChangedCB) 
					onChangedCB(data); 
			}
			ImGui::SameLine();
			addSamplerEditWidget(sampler, { 40, 40 }, name, [&data, onChangedCB](UUID uuid) {
				if (onChangedCB)
					onChangedCB(data);
				});
			ImGui::PopID();
		}

	}

	if (data.getMaterialRenderMode() == MaterialRenderMode::Terrain && ImGui::CollapsingHeader("Terrain Layers"))
	{
		const ImVec2 texSize{ 40, 40 };

		int layerIndex = 0;
		for (auto& [name, value] : data.getAllProptiesOfType(MaterialPropertyType::TERRAIN_LAYER))
		{
			auto layer = std::get<std::shared_ptr<TerrainLayerAsset>>(value);

			ImGui::PushID((name + std::to_string(layerIndex)).c_str());
			ImGui::Indent();

			if (ImGui::CollapsingHeader(name.c_str()))
			{
				ImGui::Indent();

				ImGui::Text("Albedo");
				addTextureEditWidget("Albedo", layer->albedoTexture, texSize, [&data, name, layer, onChangedCB](UUID uuid) {
					layer->albedoTexture = TextureAssetRef(uuid);
					data.setProperty(name, layer);
					if (onChangedCB) onChangedCB(data);
				});

				ImGui::Text("Normal");
				addTextureEditWidget("Normal", layer->normalTexture, texSize, [&data, name, layer, onChangedCB](UUID uuid) {
					layer->normalTexture = TextureAssetRef(uuid);
					data.setProperty(name, layer);
					if (onChangedCB) onChangedCB(data);
				});

				ImGui::Text("Metallic");
				addTextureEditWidget("Metallic", layer->metallicTexture, texSize, [&data, name, layer, onChangedCB](UUID uuid) {
					layer->metallicTexture = TextureAssetRef(uuid);
					data.setProperty(name, layer);
					if (onChangedCB) onChangedCB(data);
				});

				ImGui::Text("Roughness");
				addTextureEditWidget("Roughness", layer->roughnessTexture, texSize, [&data, name, layer, onChangedCB](UUID uuid) {
					layer->roughnessTexture = TextureAssetRef(uuid);
					data.setProperty(name, layer);
					if (onChangedCB) onChangedCB(data);
				});

				ImGui::Text("AO");
				addTextureEditWidget("AO", layer->aoTexture, texSize, [&data, name, layer, onChangedCB](UUID uuid) {
					layer->aoTexture = TextureAssetRef(uuid);
					data.setProperty(name, layer);
					if (onChangedCB) onChangedCB(data);
				});

				ImGui::Unindent();
			}
			ImGui::Unindent();

			ImGui::PopID();
		}
		layerIndex++;
	}

	// Display Uniforms and Update Shader
	if (ImGui::CollapsingHeader("Uniforms"))
	{
		for (auto& [name, value] : data.getAllProperties())
		{
			if (std::holds_alternative<std::shared_ptr<TextureSamplerAsset>>(value))
				continue;

			ImGui::PushID(name.c_str());
			bool updated = false; // Track if the value was changed
			std::visit([&](auto& v)
				{
					using T = std::decay_t<decltype(v)>;
					ImGui::Text("%s:", name.c_str());

					if constexpr (std::is_same_v<T, float>)
					{
						updated = ImGui::DragFloat(("##" + name).c_str(), &v, 0.1f);
					}
					else if constexpr (std::is_same_v<T, glm::vec2>)
					{
						updated = ImGui::DragFloat2(("##" + name).c_str(), &v[0], 0.1f);
					}
					else if constexpr (std::is_same_v<T, glm::vec3>)
					{
						updated = ImGui::DragFloat3(("##" + name).c_str(), &v[0], 0.1f);
					}
					else if constexpr (std::is_same_v<T, glm::vec4>)
					{
						updated = ImGui::DragFloat4(("##" + name).c_str(), &v[0], 0.1f);
					}
					else if constexpr (std::is_same_v<T, int>)
					{
						updated = ImGui::InputInt(("##" + name).c_str(), &v);
					}
					else if constexpr (std::is_same_v<T, unsigned int>)
					{
						updated = ImGui::InputScalar(("##" + name).c_str(), ImGuiDataType_U32, &v);
					}
					else if constexpr (std::is_same_v<T, glm::mat3>)
					{
						for (int i = 0; i < 3; ++i)
							updated |= ImGui::DragFloat3((name + "##row" + std::to_string(i)).c_str(), &v[i][0], 0.1f);
					}
					else if constexpr (std::is_same_v<T, glm::mat4>)
					{
						for (int i = 0; i < 4; ++i)
							updated |= ImGui::DragFloat4((name + "##row" + std::to_string(i)).c_str(), &v[i][0], 0.1f);
					}
				}, value);

			// If the value changed, update the shader
			if (updated)
			{
				data.setProperty(name,value);
				if (onChangedCB) 
					onChangedCB(data);
			}

			ImGui::PopID();
		}
	}

}

