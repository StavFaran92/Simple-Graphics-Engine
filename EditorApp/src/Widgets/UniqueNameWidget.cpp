#include "UniqueNameWidget.h"
#include "EditorState.h"

#include "sge.h"

#include <imgui_stdlib.h>

UniqueNameWidget::UniqueNameWidget(const std::string& label)
	: m_label(label)
{
}

// Why the name can't be used, or empty if it can
static std::string nameProblem(const std::string& name)
{
	const std::string error = UniqueNameManager::fileNameError(name);
	if (!error.empty())
		return "Name " + error;
	if (Engine::get()->getSubSystem<UniqueNameManager>()->isNameExists(name, EditorState::Instance().getWorkingDir().path()))
		return "Name is taken";
	return "";
}

bool UniqueNameWidget::isValid() const
{
	return nameProblem(name).empty();
}

void UniqueNameWidget::draw()
{
	ImGui::InputText("Name", &name);

	const std::string problem = nameProblem(name);
	if (!problem.empty())
	{
		// TODO fix, font is blurry

		ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 80, 80, 255)); // soft red
		//ImGui::SetWindowFontScale(0.9f);                                 // a bit smaller

		ImGui::TextUnformatted(("*" + problem).c_str());

		//ImGui::SetWindowFontScale(1.0f);                                 // restore
		ImGui::PopStyleColor();
	}
}

void UniqueNameWidget::clear()
{

	name = "";
}