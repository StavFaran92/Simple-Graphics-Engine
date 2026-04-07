#pragma once

#include "sge.h"

#include "DialogBase.h"
#include "Widgets/UniqueNameWidget.h"

class ShaderCreateDialog : public DialogBase
{
public:
	ShaderCreateDialog();
	void appearContent() override;
	void drawContent() override;
	bool acceptContent() override;
	void cancelContent() override;

private:
	UniqueNameWidget m_uniqueName{ "Name" };
	ShaderOverride m_shaderOverrideType = ShaderOverride::None;
};
