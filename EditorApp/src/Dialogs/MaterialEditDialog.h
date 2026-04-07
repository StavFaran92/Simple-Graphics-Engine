#pragma once

#include "DialogBase.h"
#include "Widgets.h"

class MaterialEditDialog : public DialogBase
{
public:
	MaterialEditDialog();
	void appearContent() override;
	void drawContent() override;
	bool acceptContent() override;
	void cancelContent() override;

private:
	MaterialData m_previousMaterialData;
	MaterialDataWidget m_materialData;
};
