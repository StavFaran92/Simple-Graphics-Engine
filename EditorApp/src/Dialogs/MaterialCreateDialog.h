#pragma once

#include "DialogBase.h"
#include "Widgets.h"

#include "Widgets/UniqueNameWidget.h"
#include "Widgets/FilepathWidget.h"

class MaterialCreateDialog : public DialogBase
{
public:
	MaterialCreateDialog();
	// Inherited via DialogBase
	void appearContent() override;
	void drawContent() override;
	bool acceptContent() override;
	void cancelContent() override;

	UniqueNameWidget m_uniqueName{ "Name" };
	ResourceWrapper<Material> m_tempMaterial;
	MaterialDataWidget m_matData;
};