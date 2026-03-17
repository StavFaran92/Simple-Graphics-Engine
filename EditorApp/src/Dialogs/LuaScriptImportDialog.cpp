#include "LuaScriptImportDialog.h"

#include "EditorState.h"
#include "memory/Assets.h"

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
		uniqueName.name = Engine::get()->getSubSystem<UniqueNameManager>()->suggestUniqueName(filename, EditorState::Instance().getWorkingDir().path());
	}
}

bool LuaScriptImportDialog::acceptContent()
{
	if (uniqueName.isValid())
	{
		AssetBuildDescriptor desc;
		desc.aType = AssetType::LUA_SCRIPT;
		desc.name = uniqueName.name;
		desc.sourcePath = filepath.m_filepath;
		desc.targetDirectory = EditorState::Instance().getWorkingDir().path();
		LuaScriptLoadDescriptor loadDesc;
		loadDesc.sourcePath = filepath.m_filepath;
		Engine::get()->getSubSystem<Assets>()->importAsset(desc, loadDesc);

		return true;
	}
	return false;
}

void LuaScriptImportDialog::cancelContent()
{
}
