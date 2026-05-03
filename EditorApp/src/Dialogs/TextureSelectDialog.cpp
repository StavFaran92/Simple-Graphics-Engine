#include "TextureSelectDialog.h"

#include "EditorState.h"
#include "memory/Assets.h"
#include "texture/Texture.h"
#include "imgui.h"

TextureSelectDialog::TextureSelectDialog()
	: DialogBase("TextureSelectDialog")
{
}

void TextureSelectDialog::appearContent()
{
	m_selectedTextureIndex = -1;
}

void TextureSelectDialog::drawContent()
{
	auto assets = Engine::get()->getSubSystem<Assets>();
	const auto& textureRecordList = assets->getAllRecordsOfType(AssetType::TEXTURE);

	ImGui::Text("Available Textures:");
	ImGui::Separator();

	if (m_selectedTextureIndex != -1 && m_selectedTextureIndex < static_cast<int>(textureRecordList.size()))
	{
		TextureAssetRef textureHandle(textureRecordList.at(m_selectedTextureIndex)->uuid);
		TextureResourceRef displayTexture = textureHandle.resource();
		ImGui::Image(reinterpret_cast<ImTextureID>(displayTexture.get()->getID()), ImVec2(150, 150), ImVec2(0, 1), ImVec2(1, 0));
	}

	ImGui::Separator();

	for (int i = 0; i < static_cast<int>(textureRecordList.size()); i++)
	{
		bool isSelected = (m_selectedTextureIndex == i);
		if (isSelected)
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
		}
		else
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
		}

		if (ImGui::Selectable(textureRecordList[i]->name.c_str(), false))
		{
			m_selectedTextureIndex = i;
		}
		ImGui::PopStyleColor();
	}
}

bool TextureSelectDialog::acceptContent()
{
	auto assets = Engine::get()->getSubSystem<Assets>();
	const auto& textureRecordList = assets->getAllRecordsOfType(AssetType::TEXTURE);
	if (m_selectedTextureIndex >= 0 && m_selectedTextureIndex < static_cast<int>(textureRecordList.size()))
	{
		if (onAcceptCB)
		{
			onAcceptCB(textureRecordList[m_selectedTextureIndex]->uuid);
		}
	}
	return true;
}

void TextureSelectDialog::cancelContent()
{
	m_selectedTextureIndex = -1;
}
