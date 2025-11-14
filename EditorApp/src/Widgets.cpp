#include "Widgets.h"

#include "Dialogs.h"
#include "EditorState.h"
#include <imgui_stdlib.h>
#include "tinyfiledialogs.h"

void addTextureEditWidget(int textureID, ImVec2 size, std::function<void(UUID uuid)> callback)
{
	if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(textureID), size))
	{
		ImGui::OpenPopup("EditTexturePopup");
		EditorState::Instance().assetTextureSelectCB = callback;
	}

	displayTextureSelectDialog();
}

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

void addTextureEditWidget(AssetWrapper<Material> mat, const std::string& name, Texture::TextureType ttype)
{
	AssetWrapper<Texture> tex = AssetWrapper<Texture>::empty;
	if (mat.resource()->hasTexture(ttype))
	{
		tex = mat.resource()->getSampler(ttype)->texture;
	}

	addTextureEditWidget(tex, { 20, 20 }, [=](UUID uuid) {
		mat.get()->setTexture(ttype, AssetWrapper<Texture>(uuid));
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

FilepathWidget::FilepathWidget(const std::string& label, char const* const* formats, size_t formatCount)
	: m_label(label), m_formats(formats), m_formatCount(formatCount)
{
}

UniqueNameWidget::UniqueNameWidget(const std::string& label)
	: m_label(label)
{
}

bool UniqueNameWidget::isValid() const
{
	if (Engine::get()->getSubSystem<UniqueNameManager>()->isNameExists(name))
	{
		return false;
	}
	else if (name.empty())
	{
		return false;
	}
	return true;
}

void UniqueNameWidget::draw()
{
	ImGui::InputText("Name", &name);

	if (!isValid())
	{
		// TODO fix, font is blurry

		ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 80, 80, 255)); // soft red
		//ImGui::SetWindowFontScale(0.9f);                                 // a bit smaller

		ImGui::TextUnformatted("*Name is taken or empty");

		//ImGui::SetWindowFontScale(1.0f);                                 // restore
		ImGui::PopStyleColor();
	}
}

void UniqueNameWidget::clear()
{

	name = "";
}

bool FilepathWidget::isValid() const
{
	return std::filesystem::exists(m_filepath);
}

void FilepathWidget::draw()
{
	ImGui::InputText(m_label.c_str(), &m_filepath);
	ImGui::SameLine();
	m_isPressed = ImGui::Button("o");
	if(m_isPressed)
	{
		const char* filepath = tinyfd_openFileDialog(
			"Select an asset to load",
			"",
			m_formatCount,
			m_formats,
			"",
			0);

		if (filepath)
		{
			m_filepath = filepath;
		}
	}
}

void FilepathWidget::clear()
{
	m_filepath = "";
	m_isPressed = false;
}

bool FilepathWidget::accept()
{
	bool tempIsPressed = m_isPressed;
	m_isPressed = false;
	return tempIsPressed;
}


void TextureDataWidget::draw()
{
	// --- Filter type ---
	static const char* filterModes[] = {
		"Nearest",
		"Linear",
		"Nearest Mip Nearest",
		"Linear Mip Nearest",
		"Nearest Mip Linear",
		"Linear Mip Linear"
	};

	ImGui::Text("Filter Mode");
	ImGui::Combo("##FilterMode", (int*)&m_filterMode, filterModes, IM_ARRAYSIZE(filterModes));

	// --- Anisotropy ---
	ImGui::Text("Anisotropy");
	ImGui::SliderFloat("##Aniso", &m_anisotropy, 1.0f, 16.0f);

	// --- Wrap mode (the one you meant: repeat / clamp-to-edge / clamp-to-border / mirrored-repeat) ---
	static const char* wrapModes[] = {
		"Repeat",
		"Clamp to Edge",
		"Clamp to Border",
		"Mirrored Repeat"
	};

	ImGui::Text("Wrap Mode");
	ImGui::Combo("##WrapMode", (int*)&m_wrapMode, wrapModes, IM_ARRAYSIZE(wrapModes));
}

void MaterialDataWidget::draw(const ResourceWrapper<Material>& mat)
{
	ImGui::Text(mat.get()->getName().c_str());

	ImGui::Dummy(ImVec2(0, 4));

	ImGui::ColorEdit3("Base Color", glm::value_ptr(mat.get()->colorDiffuse));
	ImGui::DragFloat("Metallic", &mat.get()->metallicFactor, 0.01f, 0.0f, 1.0f);
	ImGui::DragFloat("Roughness", &mat.get()->roughnessFactor, 0.01f, 0.0f, 1.0f);
	ImGui::DragFloat("Opacity", &mat.get()->opacityFactor, 0.01f, 0.0f, 1.0f);

	addSamplerEditWidget(mat, { 40, 40 }, "Albedo", Texture::TextureType::Albedo);
	addSamplerEditWidget(mat, { 40, 40 }, "Normal", Texture::TextureType::Normal);
	addSamplerEditWidget(mat, { 40, 40 }, "Metallic", Texture::TextureType::Metallic);
	addSamplerEditWidget(mat, { 40, 40 }, "Roughness", Texture::TextureType::Roughness);
	addSamplerEditWidget(mat, { 40, 40 }, "Ambient Occlusion", Texture::TextureType::AmbientOcclusion);
}
