#include "AssetInspectorWindow.h"

#include "EditorState.h"
#include "Widgets.h"

void AssetInspectorWindow::display()
{
	ImGui::Begin("AssetInspector", nullptr, windowFlags &
		~ImGuiWindowFlags_NoScrollbar &
		~ImGuiWindowFlags_NoScrollWithMouse);

	auto& currentAssetEdit = EditorState::Instance().currentAssetEdit;
	if (currentAssetEdit.isEmpty())
	{
		ImGui::TextDisabled("No asset selected");
		ImGui::End();
		return;
	}

	const AssetRecord& info = currentAssetEdit.info();
	ImGui::Text("Name: %s", info.name.c_str());
	ImGui::Text("Type: %s", getAssetTypeAsStr(info.aType).c_str());
	ImGui::Separator();

	switch (info.aType)
	{
	case AssetType::MATERIAL:
	{
		MaterialAssetRef material = currentAssetEdit.as<MaterialAsset>();
		if (material.isEmpty())
		{
			ImGui::TextColored(ImVec4(1.f, 0.3f, 0.3f, 1.f), "Failed to load material.");
			break;
		}

		static MaterialDataWidget materialWidget;
		materialWidget.draw(material->data, [&material](const MaterialData&) {
			material.makeDirty();
		});
		break;
	}
	default:
		ImGui::TextDisabled("Asset type not supported yet.");
		break;
	}

	ImGui::End();
}
