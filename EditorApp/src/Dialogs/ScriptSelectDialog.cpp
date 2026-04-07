#include "ScriptSelectDialog.h"

#include "EditorState.h"
#include "NativeScriptsLoader.h"
#include "imgui.h"

ScriptSelectDialog::ScriptSelectDialog()
	: DialogBase("ScriptSelectDialog")
{
}

void ScriptSelectDialog::appearContent()
{
	m_selectedScriptIndex = -1;
	m_scriptNames.clear();
	NativeScriptsLoader::instance->getAllScripts(m_scriptNames);
}

void ScriptSelectDialog::drawContent()
{
	ImGui::Text("Available Scripts:");
	ImGui::Separator();

	for (int i = 0; i < static_cast<int>(m_scriptNames.size()); i++)
	{
		bool isSelected = (m_selectedScriptIndex == i);
		if (ImGui::Selectable(m_scriptNames[i].c_str(), &isSelected))
		{
			m_selectedScriptIndex = i;
		}
	}
}

bool ScriptSelectDialog::acceptContent()
{
	if (m_selectedScriptIndex >= 0 && m_selectedScriptIndex < static_cast<int>(m_scriptNames.size()))
	{
		if (EditorState::Instance().scriptSelectCB)
		{
			EditorState::Instance().scriptSelectCB(m_scriptNames[m_selectedScriptIndex]);
		}
	}
	return true;
}

void ScriptSelectDialog::cancelContent()
{
}
