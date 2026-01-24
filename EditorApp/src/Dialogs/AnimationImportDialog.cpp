#include "AnimationImportDialog.h"
#include "EditorState.h"

AnimationImportDialog::AnimationImportDialog()
	: DialogBase("AnimationImportDialog")
{
}

void AnimationImportDialog::appearContent()
{
	uniqueName.clear();
	filepath.clear();
}

void AnimationImportDialog::drawContent()
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
}

bool AnimationImportDialog::acceptContent()
{
	if (uniqueName.isValid())
	{
		AnimationImportSettings desc;
		desc.name = uniqueName.name;
		desc.targetDirectory = EditorState::Instance().getWorkingDir().path();
		AnimationAsset::import(filepath.m_filepath, desc);

		return true;
	}
	return false;
}

void AnimationImportDialog::cancelContent()
{
}
