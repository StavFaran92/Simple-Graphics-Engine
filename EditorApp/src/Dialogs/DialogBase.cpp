#include "DialogBase.h"

#include "imgui.h"

#include "EditorState.h"

DialogBase::DialogBase(const std::string& label)
	: m_label(label)
{
	EditorState::Instance().addDialogDisplay(this);
}

void DialogBase::header()
{
	headerContent();
}

void DialogBase::footer()
{
	footerContent();

	if (ImGui::Button("OK", ImVec2(120, 0)))
	{
		if (acceptContent())
		{
			EditorState::Instance().setState(m_label, false);
		}
	}
	ImGui::SameLine();

	if (ImGui::Button("Cancel", ImVec2(120, 0)))
	{
		cancelContent();
		EditorState::Instance().setState(m_label, false);
	}

	ImGui::End();
}

void DialogBase::activate()
{
	EditorState::Instance().setState(m_label, true);
}

void DialogBase::draw()
{
	// Center dialog
	ImGui::SetNextWindowPos(
		ImGui::GetMainViewport()->GetCenter(),
		ImGuiCond_Appearing,
		ImVec2(0.5f, 0.5f));

	if (EditorState::Instance().getState(m_label) && ImGui::Begin(m_label.c_str(), nullptr, 
		ImGuiWindowFlags_AlwaysAutoResize  | 
		ImGuiWindowFlags_NoDocking | 
		//ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoCollapse))
	{
		if (ImGui::IsWindowAppearing())
		{
			appearContent();
		}
		header();
		drawContent();
		footer();
	}
}

void DialogBase::display()
{
	draw();
}