#include "AnimationImportDialog.h"
#include "EditorState.h"
#include "memory/Assets.h"

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
		AssetCreateDescriptor desc;
		desc.aType = AssetType::ANIMATION;
		desc.name = uniqueName.name;
		desc.sourcePath = filepath.m_filepath;
		desc.targetDirectory = EditorState::Instance().getWorkingDir().path();
		Engine::get()->getSubSystem<Assets>()->importAsset(desc);

		return true;
	}
	return false;
}

void AnimationImportDialog::cancelContent()
{
}
