#include "FolderCreateDialog.h"
#include "EditorState.h"

FolderCreateDialog::FolderCreateDialog()
	: DialogBase("FolderCreateDialog")
{
}

void FolderCreateDialog::appearContent()
{
	m_uniqueName.name = Engine::get()->getSubSystem<UniqueNameManager>()->suggestUniqueName("New Folder", EditorState::Instance().getWorkingDir().path());
}

void FolderCreateDialog::drawContent()
{
	m_uniqueName.draw();
}

bool FolderCreateDialog::acceptContent()
{
	if (m_uniqueName.isValid())
	{
		auto path = EditorState::Instance().getWorkingDir().path().absolute() / m_uniqueName.name;
		std::filesystem::create_directories(path);
		return true;
	}
	return false;
}

void FolderCreateDialog::cancelContent()
{
}
