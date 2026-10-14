#include "AssetSelectDialog.h"
#include "EditorState.h"
#include "imgui.h"
#include "Widgets/AssetCard.h"

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
	// The dialog auto-resizes, so the grid gets a fixed size region instead of taking the window width
	const int visibleColumns = 5;
	const int visibleRows = 3;
	const ImVec2 cardSize = AssetCard::getSize();
	const ImGuiStyle& style = ImGui::GetStyle();
	const ImVec2 regionSize(
		visibleColumns * (cardSize.x + AssetCard::Spacing) - AssetCard::Spacing + style.WindowPadding.x * 2.0f + style.ScrollbarSize,
		visibleRows * (cardSize.y + AssetCard::Spacing) - AssetCard::Spacing + style.WindowPadding.y * 2.0f);

	ImGui::BeginChild("AssetSelectGrid", regionSize, true);

	if (currentDisplayedAssetList.empty())
		ImGui::TextDisabled("No %s assets found", getAssetTypeAsStr(assetType).c_str());

	AssetCardGrid grid;
	grid.begin();

	for (int i = 0; i < currentDisplayedAssetList.size(); i++)
	{
		const AssetRecord& aInfo = currentDisplayedAssetList[i];

		grid.next();
		ImGui::PushID(i);

		AssetCard::draw(aInfo, selectedAssetIndex == i);

		if (ImGui::IsItemClicked())
			selectedAssetIndex = i;

		// Double click picks the asset right away, same as selecting it and pressing OK
		if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
		{
			selectedAssetIndex = i;
			accept();
		}

		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::TextUnformatted(aInfo.name.c_str());
			ImGui::TextDisabled("%s", aInfo.getScopedPath().scoped().string().c_str());
			ImGui::EndTooltip();
		}

		ImGui::PopID();
	}

	grid.end();

	ImGui::EndChild();
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
