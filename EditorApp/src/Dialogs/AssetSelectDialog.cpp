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

	const std::vector<const AssetRecord*>& fullAssetList = Engine::get()->getSubSystem<Assets>()->getAllRecordsOfType(assetType);

	//filter non visible assets
	currentDisplayedAssetList.clear();
	for (int i = 0; i < fullAssetList.size(); i++)
	{
		if (fullAssetList[i]->isVisible())
		{
			currentDisplayedAssetList.push_back(*fullAssetList[i]);
		}
	}
}

void AssetSelectDialog::drawContent()
{
	

	for (int i = 0; i < currentDisplayedAssetList.size(); i++)
	{
		ImVec2 cursorStart = ImGui::GetCursorScreenPos();
		ImVec2 totalSize = ImVec2(ImGui::GetContentRegionAvail().x, 0);



		bool isSelected = (selectedAssetIndex == i);


		// Draw normal part
		ImGui::TextUnformatted(currentDisplayedAssetList[i].name.c_str());
		ImGui::SameLine();

		// Draw gray part
		ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(128, 128, 128, 255));
		std::string grayText = "(" + currentDisplayedAssetList[i].getScopedPath().scoped().string() + ")";
		ImGui::TextUnformatted(grayText.c_str());

		ImGui::PopStyleColor();

		std::string fullText;
		if (isSelected)
		{
			fullText = currentDisplayedAssetList[i].name + grayText;
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
	if (selectedAssetIndex >= 0 && selectedAssetIndex < currentDisplayedAssetList.size())
	{
		onAccpetCB(currentDisplayedAssetList[selectedAssetIndex].uuid);
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
		if (selectedAssetIndex != -1 && selectedAssetIndex < currentDisplayedAssetList.size())
		{
			TextureAssetRef textureHandle = TextureAssetRef(currentDisplayedAssetList.at(selectedAssetIndex).uuid);
			TextureResourceRef displayTexture = textureHandle.resource();
			ImVec2 imageSize(150, 150);
			ImGui::Image(reinterpret_cast<ImTextureID>(displayTexture.get()->getID()), imageSize, ImVec2(0, 1), ImVec2(1, 0), ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 1));
		}
	}
}
