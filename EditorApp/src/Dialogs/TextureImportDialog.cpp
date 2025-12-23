#include "TextureImportDialog.h"
#include "EditorState.h"

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
		Texture::TextureAssetDescriptor desc;
		desc.name = uniqueName.name;
		desc.targetDirectory = EditorState::Instance().getWorkingDir().path();
		desc.GPUformat = Texture::InternalFormat::R32F; // todo fix
		Texture::import(filepath.m_filepath, desc);

		return true;
	}
	return false;
}

void TextureImportDialog::cancelContent()
{
}
