#include "LuaScriptCreateDialog.h"

#include "EditorState.h"
#include "memory/Assets.h"

LuaScriptCreateDialog::LuaScriptCreateDialog()
	: DialogBase("LuaScriptCreateDialog")
{
}

void LuaScriptCreateDialog::appearContent()
{
	m_uniqueName.name = Engine::get()->getSubSystem<UniqueNameManager>()->suggestUniqueName("NewLuaScript", EditorState::Instance().getWorkingDir().path());
}

void LuaScriptCreateDialog::drawContent()
{
	m_uniqueName.draw();
	ImGui::Separator();
}

bool LuaScriptCreateDialog::acceptContent()
{
	if (!m_uniqueName.isValid())
	{
		return false;
	}

	AssetBuildDescriptor desc;
	desc.aType = AssetType::LUA_SCRIPT;
	desc.name = m_uniqueName.name;
	desc.targetDirectory = EditorState::Instance().getWorkingDir().path();
	LuaScriptCreateDescriptor createDesc;
	Engine::get()->getSubSystem<Assets>()->createAsset(desc, createDesc);
	return true;
}

void LuaScriptCreateDialog::cancelContent()
{
}
