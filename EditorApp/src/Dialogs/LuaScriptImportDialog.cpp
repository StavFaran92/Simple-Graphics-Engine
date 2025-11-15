#include "LuaScriptImportDialog.h"

#include "EditorState.h"

LuaScriptImportDialog::LuaScriptImportDialog()
	: DialogBase("LuaScriptImportDialog")
{
}

void LuaScriptImportDialog::appearContent()
{
	uniqueName.clear();
	filepath.clear();
}

void LuaScriptImportDialog::drawContent()
{
	uniqueName.draw();

	filepath.draw();
	if (filepath.accept())
	{
		std::filesystem::path path(filepath.m_filepath);
		std::string filename = path.filename().stem().string();
		uniqueName.name = Engine::get()->getSubSystem<UniqueNameManager>()->suggestUniqueName(filename, EditorState::Instance().getWorkingDir().string());
	}
}

bool LuaScriptImportDialog::acceptContent()
{
	if (uniqueName.isValid())
	{
		LuaScriptImportSettings desc;
		desc.name = uniqueName.name;
		LuaScript::import(filepath.m_filepath, desc);

		return true;
	}
	return false;
}

void LuaScriptImportDialog::cancelContent()
{
}
