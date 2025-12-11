#include "Widgets.h"

#include "Dialogs.h"
#include "EditorState.h"
#include <imgui_stdlib.h>
#include "tinyfiledialogs.h"
#include "dialogs/AssetSelectDialog.h"

void addTextureEditWidget(AssetWrapper<Texture> texture, ImVec2 size, std::function<void(UUID uuid)> callback)
{
	int texID = 0;
	if (!texture.isEmpty())
	{
		texID = texture.get()->getID();
	}

	if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(texID), size))
	{
		ImGui::OpenPopup("EditTexturePopup");
		EditorState::Instance().assetTextureSelectCB = callback;
	}

	displayTextureSelectDialog();
}

void addSamplerEditWidget(std::shared_ptr<TextureSampler> sampler, ImVec2 size, const std::string& name)
{
	ImGui::PushID(name.c_str());

	int texID = 0;
	if (!sampler->texture.isEmpty())
	{
		texID = sampler->texture.get()->getID();
	}

	if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(texID), size))
	{
		ImGui::OpenPopup("EditSamplerPopup");
		EditorState::Instance().selectedSampler = sampler;
		EditorState::Instance().previousSampler = std::make_shared<TextureSampler>(*sampler.get());
	}

	if (ImGui::BeginPopup("EditSamplerPopup"))
	{
		if (!EditorState::Instance().selectedSampler)
		{
			logError("Selected sampler cannot be null.");
			ImGui::EndPopup();
			return;
		}
		auto assets = Engine::get()->getSubSystem<Assets>();

		ImGui::Text("Texture");
		addTextureEditWidget(EditorState::Instance().selectedSampler->texture, ImVec2{ 150, 150 }, [=](UUID uuid) {
			EditorState::Instance().selectedSampler->texture = AssetWrapper<Texture>(uuid);
			});

		ImGui::Spacing();

		static int* currentChannelMask[4];

		currentChannelMask[0] = &EditorState::Instance().selectedSampler->channelMaskR;
		currentChannelMask[1] = &EditorState::Instance().selectedSampler->channelMaskG;
		currentChannelMask[2] = &EditorState::Instance().selectedSampler->channelMaskB;
		currentChannelMask[3] = &EditorState::Instance().selectedSampler->channelMaskA;

		for (int i = 0; i < EditorState::Instance().selectedSampler->channelCount; i++)
		{
			ImGui::PushID(&currentChannelMask[i]);
			displayChannelSelectWidget(currentChannelMask[i]);
			ImGui::PopID();
		}

		ImGui::Spacing();

		ImGui::DragFloat("xoffset", &EditorState::Instance().selectedSampler->xOffset, .1f);
		ImGui::DragFloat("yoffset", &EditorState::Instance().selectedSampler->yOffset, .1f);

		ImGui::Spacing();

		ImGui::DragFloat("xScale", &EditorState::Instance().selectedSampler->xScale, .1f);
		ImGui::DragFloat("yScale", &EditorState::Instance().selectedSampler->yScale, .1f);

		ImGui::Separator();

		if (ImGui::Button("OK"))
		{
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel"))
		{
			*sampler = *EditorState::Instance().previousSampler;
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
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

// TODO fix
extern AssetSelectDialog assetSelectDialog;

bool addAssetSelectWidget(const std::string& name, AssetType aType, const std::function<void(UUID)>& onAccpetCB)
{
	float width = ImGui::GetContentRegionAvail().x;

	if (ImGui::Button(name.c_str(), ImVec2(width, 0)))
	{
		assetSelectDialog.assetType = aType;
		assetSelectDialog.onAccpetCB = onAccpetCB;
		assetSelectDialog.activate();
	}

	return false;
}




void MaterialDataWidget::draw(const ResourceWrapper<Material>& mat)
{
	ImGui::Text(mat.get()->getName().c_str());

	ImGui::Dummy(ImVec2(0, 4));

	// TODO replace using magic enum
	static const char* RenderModeNames[] = {
		"Opaque",
		"Transparent",
		"Terrain",
		"Skybox",
		"Unlit",
		"UI",
		"Custom"
	};

	static MaterialRenderMode currentMode;
	currentMode = mat.get()->getMaterialRenderMode();
	int currentIndex = static_cast<int>(currentMode);

	if (ImGui::BeginCombo("Render Mode", RenderModeNames[currentIndex])) {
		for (int i = 0; i < IM_ARRAYSIZE(RenderModeNames); ++i) {
			bool isSelected = (i == currentIndex);
			if (ImGui::Selectable(RenderModeNames[i], isSelected)) {
				currentIndex = i;
				currentMode = static_cast<MaterialRenderMode>(i);
				mat.get()->setMaterialRenderMode(currentMode);
			}
			if (isSelected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	if (currentMode == MaterialRenderMode::Custom)
	{
		std::string shaderName = "None";
		if (!mat.get()->m_shader.isEmpty())
		{
			shaderName = mat.get()->m_shader.info().name;
		}

		addAssetSelectWidget(shaderName, AssetType::SHADER, [mat](UUID uuid) {
			mat.get()->m_shader = AssetWrapper<Shader>(uuid);
		});
	}

	// Custom Textures Array
	if (ImGui::CollapsingHeader("Samplers"))
	{
		for (auto& [name, sampler] : mat.get()->m_samplers)
		{
			ImGui::PushID(name.c_str());
			ImGui::Text(name.c_str());
			ImGui::Checkbox("", (bool*)&sampler->isActive);
			ImGui::SameLine();
			addSamplerEditWidget(sampler, { 40, 40 }, name);
			ImGui::PopID();
		}

	}

	// Display Uniforms and Update Shader
	if (ImGui::CollapsingHeader("Uniforms"))
	{
		for (auto& [name, uniform] : mat.get()->m_uniformProperties)
		{
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
				}, uniform.value);

			// If the value changed, update the shader
			if (updated)
			{
				mat.get()->setUniformValue(name, uniform.value);
			}

			ImGui::PopID();
		}
	}

	//ImGui::ColorEdit3("Base Color", glm::value_ptr(mat.get()->colorDiffuse));
	//ImGui::DragFloat("Metallic", &mat.get()->metallicFactor, 0.01f, 0.0f, 1.0f);
	//ImGui::DragFloat("Roughness", &mat.get()->roughnessFactor, 0.01f, 0.0f, 1.0f);
	//ImGui::DragFloat("Opacity", &mat.get()->opacityFactor, 0.01f, 0.0f, 1.0f);

	//addSamplerEditWidget(mat, { 40, 40 }, "Albedo", Texture::TextureType::Albedo);
	//addSamplerEditWidget(mat, { 40, 40 }, "Normal", Texture::TextureType::Normal);
	//addSamplerEditWidget(mat, { 40, 40 }, "Metallic", Texture::TextureType::Metallic);
	//addSamplerEditWidget(mat, { 40, 40 }, "Roughness", Texture::TextureType::Roughness);
	//addSamplerEditWidget(mat, { 40, 40 }, "Ambient Occlusion", Texture::TextureType::AmbientOcclusion);
}
