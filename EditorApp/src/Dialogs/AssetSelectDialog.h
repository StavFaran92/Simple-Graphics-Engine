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

	UUID getSelectedUUID() const;
	void setType(AssetType aType);

private:
	int selectedAssetIndex = -1;
	AssetType assetType = AssetType::NONE;
	UUID selectedUUID = EMPTY_UUID;
};