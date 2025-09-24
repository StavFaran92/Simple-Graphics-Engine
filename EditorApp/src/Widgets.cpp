#include "Widgets.h"

#include "Dialogs.h"
#include "EditorState.h"

void addTextureEditWidget(int textureID, ImVec2 size, std::function<void(std::string uuid)> callback)
{
	if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(textureID), size))
	{
		ImGui::OpenPopup("EditTexturePopup");
		EditorState::Instance().assetTextureSelectCB = callback;
	}

	displayTextureSelectDialog();
}

void addTextureEditWidget(ResourceWrapper<Texture> texture, ImVec2 size, std::function<void(std::string uuid)> callback)
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

void addTextureEditWidget(ResourceWrapper<Material> mat, const std::string& name, Texture::TextureType ttype)
{
	ResourceWrapper<Texture> tex = ResourceWrapper<Texture>::empty;
	if (mat->hasTexture(ttype))
	{
		tex = mat->getSampler(ttype)->texture;
	}

	addTextureEditWidget(tex, { 20, 20 }, [=](std::string uuid) {
		mat.get()->setTexture(ttype, ResourceWrapper<Texture>(uuid));
		});

	ImGui::SameLine();

	ImGui::Text(name.c_str());
}

void addSamplerEditWidget(ResourceWrapper<Material> mat, ImVec2 size, const std::string& name, Texture::TextureType ttype)
{
	ImGui::PushID(name.c_str());

	int texID = 0;
	auto sampler = mat->getSampler(ttype);

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
		addTextureEditWidget(EditorState::Instance().selectedSampler->texture, ImVec2{ 150, 150 }, [=](std::string uuid) {
			EditorState::Instance().selectedSampler->texture = ResourceWrapper<Texture>(uuid);
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
			mat->setSampler(ttype, EditorState::Instance().previousSampler);
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

void addAssetSelectWidget(const std::string& name, AssetType aType, const std::function<void(UUID)>& cb)
{
	float width = ImGui::GetContentRegionAvail().x;

	if (ImGui::Button(name.c_str(), ImVec2(width, 0)))
	{
		EditorState::Instance().showAssetSelectorWindow = true;
		EditorState::Instance().assetSelectType = aType;
		EditorState::Instance().assetSelectCB = cb;
	}
}
