#include "FilepathWidget.h"

#include "imgui.h"
#include <imgui_stdlib.h>
#include "tinyfiledialogs.h"

FilepathWidget::FilepathWidget(const std::string& label, char const* const* formats, size_t formatCount)
	: m_label(label), m_formats(formats), m_formatCount(formatCount)
{
}



bool FilepathWidget::isValid() const
{
	return std::filesystem::exists(m_filepath);
}

void FilepathWidget::draw()
{
	ImGui::InputText(m_label.c_str(), &m_filepath);
	ImGui::SameLine();
	m_isPressed = ImGui::Button("o");
	if (m_isPressed)
	{
		const char* filepath = tinyfd_openFileDialog(
			"Select an asset to load",
			"",
			m_formatCount,
			m_formats,
			"",
			0);

		if (filepath)
		{
			m_filepath = filepath;
		}
	}
}

void FilepathWidget::clear()
{
	m_filepath = "";
	m_isPressed = false;
}

bool FilepathWidget::accept()
{
	bool tempIsPressed = m_isPressed;
	m_isPressed = false;
	return tempIsPressed;
}