#include "ProjectSettingsDialog.h"

#include "sge.h"
#include <imgui.h>

#include "render/Graphics.h"

ProjectSettingsDialog::ProjectSettingsDialog()
	: DialogBase("ProjectSettingsDialog")
{
}

void ProjectSettingsDialog::appearContent()
{
}

void ProjectSettingsDialog::drawContent()
{
	auto graphics = Engine::get()->getSubSystem<Graphics>();
	ImGui::Checkbox("SSAO", &graphics->useSSAO);
}

bool ProjectSettingsDialog::acceptContent()
{
	return true;
}

void ProjectSettingsDialog::cancelContent()
{
}
