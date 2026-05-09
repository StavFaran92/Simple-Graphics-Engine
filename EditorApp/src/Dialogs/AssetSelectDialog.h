#pragma once

#include "DialogBase.h"

#include "sge.h"

class AssetSelectDialog : public DialogBase
{
public:
	AssetSelectDialog();
	// Inherited via DialogBase
	void appearContent() override;
	void drawContent() override;
	bool acceptContent() override;
	void cancelContent() override;
	void headerContent() override;

public:
	int selectedAssetIndex = -1;
	AssetType assetType = AssetType::NONE;
	std::function<void(UUID)> onAccpetCB;
	std::vector<AssetRecord> currentDisplayedAssetList;
};