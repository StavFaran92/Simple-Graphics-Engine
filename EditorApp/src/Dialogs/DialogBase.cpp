#include "DialogBase.h"

#include "imgui.h"

#include "EditorState.h"

DialogBase::DialogBase(const std::string& label)
	: m_label(label)
{
	EditorState::Instance().addDialogDisplay(this);
}

void DialogBase::appear()
{
	if (EditorState::Instance().getState(m_label))
	{
		appearContent();
		ImGui::OpenPopup(m_label.c_str());
		EditorState::Instance().setState(m_label, false);
	}
}

void DialogBase::header()
{
}

void DialogBase::footer()
{
	if (ImGui::Button("OK", ImVec2(120, 0)))
	{
		if (acceptContent())
		{
			ImGui::CloseCurrentPopup();
		}
	}
	ImGui::SameLine();

	if (ImGui::Button("Cancel", ImVec2(120, 0)))
	{
		ImGui::CloseCurrentPopup();
	}

	ImGui::EndPopup();
}

void DialogBase::activate()
{
	EditorState::Instance().setState(m_label, true);
}

void DialogBase::draw()
{
	if (ImGui::BeginPopupModal(m_label.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		header();
		drawContent();
		footer();
	}
}

void DialogBase::display()
{
	appear();
	draw();
}