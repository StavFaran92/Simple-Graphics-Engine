#include "AssetSelectDialog.h"
#include "EditorState.h"
#include "imgui.h"

AssetSelectDialog::AssetSelectDialog() :
	DialogBase("AssetSelectDialog")
{
}

void AssetSelectDialog::appearContent()
{
	selectedAssetIndex = -1;
}

void AssetSelectDialog::drawContent()
{
	const std::vector<AssetInfo>& assetList = Engine::get()->getSubSystem<Assets>()->getAllAssetsOfType(assetType);

	for (int i = 0; i < assetList.size(); i++)
	{
		ImVec2 cursorStart = ImGui::GetCursorScreenPos();
		ImVec2 totalSize = ImVec2(ImGui::GetContentRegionAvail().x, 0);



		bool isSelected = (selectedAssetIndex == i);


		// Draw normal part
		ImGui::TextUnformatted(assetList[i].name.c_str());
		ImGui::SameLine();

		// Draw gray part
		ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(128, 128, 128, 255));
		std::string grayText = "(" + assetList[i].relativefilePath + ")";
		ImGui::TextUnformatted(grayText.c_str());

		ImGui::PopStyleColor();

		std::string fullText;
		if (isSelected)
		{
			fullText = assetList[i].name + grayText;
		}
		else
		{
			fullText = "##select_" + std::to_string(i);
		}

		// Create invisible selectable
		ImGui::SetCursorScreenPos(cursorStart);
		bool selected = ImGui::Selectable(fullText.c_str(), isSelected, ImGuiSelectableFlags_DontClosePopups, totalSize);
		if (selected) {
			selectedAssetIndex = i;
		}
	}
}

bool AssetSelectDialog::acceptContent()
{
	const std::vector<AssetInfo>& assetList = Engine::get()->getSubSystem<Assets>()->getAllAssetsOfType(assetType);
	if (selectedAssetIndex >= 0 && selectedAssetIndex < assetList.size())
	{
		onAccpetCB(assetList[selectedAssetIndex].uuid);
		return true;
	}
	return false;
}

void AssetSelectDialog::cancelContent()
{
}

void AssetSelectDialog::headerContent()
{
	if (assetType == AssetType::TEXTURE)
	{
		const std::vector<AssetInfo>& assetList = Engine::get()->getSubSystem<Assets>()->getAllAssetsOfType(assetType);
		if (selectedAssetIndex != -1)
		{
			ResourceWrapper<Texture> displayTexture = assetList.at(selectedAssetIndex).resource.as<Texture>();
			ImVec2 imageSize(150, 150);
			ImGui::Image(reinterpret_cast<ImTextureID>(displayTexture.get()->getID()), imageSize, ImVec2(0, 1), ImVec2(1, 0), ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 1));
		}
	}
}
