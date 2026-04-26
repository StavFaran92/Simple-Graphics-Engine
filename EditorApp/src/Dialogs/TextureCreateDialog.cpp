#include "TextureCreateDialog.h"

#include "EditorState.h"
#include "memory/Assets.h"

TextureCreateDialog::TextureCreateDialog()
	: DialogBase("TextureCreateDialog")
{
}

void TextureCreateDialog::appearContent()
{
	m_uniqueName.name = Engine::get()->getSubSystem<UniqueNameManager>()->suggestUniqueName("NewTexture", EditorState::Instance().getWorkingDir().path());
}

void TextureCreateDialog::drawContent()
{
	m_uniqueName.draw();
	ImGui::InputInt("Width", &m_width);
	ImGui::InputInt("Height", &m_height);
	m_textureDataWidget.draw();
	ImGui::Separator();
}

bool TextureCreateDialog::acceptContent()
{
	if (!m_uniqueName.isValid())
	{
		return false;
	}

	TextureData textureData;
	textureData.target = TextureTarget::TEXTURE_2D;
	textureData.width = m_width;
	textureData.height = m_height;
	textureData.channels = 4;
	textureData.internalFormat = TextureInternalFormat::RGBA8;
	textureData.format = TextureFormat::RGBA;
	textureData.type = TextureType::UNSIGNED_BYTE;
	textureData.filter = TextureFilter::Linear;
	textureData.wrap = TextureWrap::Clamp;
	textureData.fillEmpty = true;

	AssetBuildDescriptor desc;
	desc.aType = AssetType::TEXTURE;
	desc.name = m_uniqueName.name;
	desc.targetDirectory = EditorState::Instance().getWorkingDir().path();

	TextureCreateDescriptor createDesc;
	createDesc.textureData = textureData;
	Engine::get()->getSubSystem<Assets>()->createAsset(desc, createDesc);
	return true;
}

void TextureCreateDialog::cancelContent()
{
}
