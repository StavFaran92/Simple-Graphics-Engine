#include "TextureImportDialog.h"
#include "EditorState.h"
#include "memory/Assets.h"

TextureImportDialog::TextureImportDialog()
	: DialogBase("TextureImportDialog")
{
}

void TextureImportDialog::appearContent()
{
	uniqueName.clear();
	filepath.clear();
}

void TextureImportDialog::drawContent()
{
	uniqueName.draw();

	filepath.draw();
	if (filepath.accept())
	{
		std::filesystem::path path(filepath.m_filepath);
		std::string filename = path.filename().stem().string();

		uniqueName.name = Engine::get()->getSubSystem<UniqueNameManager>()->suggestUniqueName(filename, EditorState::Instance().getWorkingDir().path());
	}
	ImGui::Separator();

	textureDataWidget.draw();
}

bool TextureImportDialog::acceptContent()
{
	if (uniqueName.isValid())
	{
		AssetBuildDescriptor desc;
		desc.name = uniqueName.name;
		desc.targetDirectory = EditorState::Instance().getWorkingDir().path();

		desc.aType = AssetType::TEXTURE;
		TextureLoadDescriptor texDesc;
		texDesc.sourcePath = filepath.m_filepath;
		texDesc.usage = textureDataWidget.m_semantic;
		Engine::get()->getSubSystem<Assets>()->importAsset(desc, texDesc);

		return true;
	}
	return false;
}

void TextureImportDialog::cancelContent()
{
}
