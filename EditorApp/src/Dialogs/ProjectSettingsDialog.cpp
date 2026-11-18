#include "ProjectSettingsDialog.h"

#include "sge.h"
#include <imgui.h>

#include "render/Graphics.h"
#include "core/ProjectSettings.h"

ProjectSettingsDialog::ProjectSettingsDialog()
	: DialogBase("ProjectSettingsDialog")
{
}

void ProjectSettingsDialog::appearContent()
{
}

void ProjectSettingsDialog::drawContent()
{
	if (!ImGui::BeginTabBar("ProjectSettingsTabs"))
		return;

	if (ImGui::BeginTabItem("Graphics"))
	{
		auto graphics = Engine::get()->getSubSystem<Graphics>();
		ImGui::Checkbox("SSAO", &graphics->useSSAO);
		ImGui::EndTabItem();
	}

	if (ImGui::BeginTabItem("Shadows"))
	{
		auto& shadows = Engine::get()->getSubSystem<Graphics>()->shadowSettings;
		ImGui::Checkbox("Enabled", &shadows.enabled);

		static const int resolutions[] = { 512, 1024, 2048, 4096 };
		static const char* resolutionNames[] = { "512", "1024", "2048", "4096" };
		int resolutionIndex = 1;
		for (int i = 0; i < IM_ARRAYSIZE(resolutions); i++)
		{
			if (resolutions[i] == shadows.resolution)
				resolutionIndex = i;
		}
		if (ImGui::Combo("Resolution", &resolutionIndex, resolutionNames, IM_ARRAYSIZE(resolutionNames)))
			shadows.resolution = resolutions[resolutionIndex];

		if (ImGui::CollapsingHeader("Cascades", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::SliderInt("Count", &shadows.cascadeCount, 1, MAX_SHADOW_CASCADES);
			ImGui::SliderFloat("Split Lambda", &shadows.splitLambda, 0.0f, 1.0f);
			ImGui::DragFloat("Distance", &shadows.shadowDistance, 1.0f, 1.0f, 10000.0f);
			ImGui::DragFloat("Caster Margin", &shadows.casterMargin, 1.0f, 0.0f, 10000.0f);
			ImGui::DragFloat("Bias", &shadows.biasScale, 0.05f, 0.0f, 20.0f);
		}

		ImGui::EndTabItem();
	}

	if (ImGui::BeginTabItem("Physics"))
	{
		if (ImGui::CollapsingHeader("Layer Masks")) 
		{
			auto& ps = ProjectSettings::get();
			ImGui::BeginChild("LayersList", ImVec2(300, 400), false);
			for (int i = 0; i < 32; i++)
			{
				char buf[128];
				std::string name = ps.getLayerName(i);
				strncpy_s(buf, name.c_str(), sizeof(buf) - 1);
				buf[sizeof(buf) - 1] = '\0';

				ImGui::PushID(i);
				ImGui::Text("Layer %2d", i + 1);
				ImGui::SameLine();
				ImGui::SetNextItemWidth(-1);
				if (ImGui::InputText("##layer", buf, sizeof(buf)))
					ps.setLayerName(i, buf);
				ImGui::PopID();
			}
			ImGui::EndChild();
			
		}
		ImGui::EndTabItem();
	}

	ImGui::EndTabBar();
}

bool ProjectSettingsDialog::acceptContent()
{
	ProjectSettings::get().save();
	return true;
}

void ProjectSettingsDialog::cancelContent()
{
}
