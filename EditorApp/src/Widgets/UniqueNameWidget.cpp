#include "UniqueNameWidget.h"

#include "sge.h"

#include <imgui_stdlib.h>

UniqueNameWidget::UniqueNameWidget(const std::string& label)
	: m_label(label)
{
}

bool UniqueNameWidget::isValid() const
{
	if (Engine::get()->getSubSystem<UniqueNameManager>()->isNameExists(name))
	{
		return false;
	}
	else if (name.empty())
	{
		return false;
	}
	return true;
}

void UniqueNameWidget::draw()
{
	ImGui::InputText("Name", &name);

	if (!isValid())
	{
		// TODO fix, font is blurry

		ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 80, 80, 255)); // soft red
		//ImGui::SetWindowFontScale(0.9f);                                 // a bit smaller

		ImGui::TextUnformatted("*Name is taken or empty");

		//ImGui::SetWindowFontScale(1.0f);                                 // restore
		ImGui::PopStyleColor();
	}
}

void UniqueNameWidget::clear()
{

	name = "";
}