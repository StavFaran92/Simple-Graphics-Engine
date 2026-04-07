#pragma once

#include "DialogBase.h"
#include "Widgets/UniqueNameWidget.h"

class LuaScriptCreateDialog : public DialogBase
{
public:
	LuaScriptCreateDialog();
	void appearContent() override;
	void drawContent() override;
	bool acceptContent() override;
	void cancelContent() override;

private:
	UniqueNameWidget m_uniqueName{ "Name" };
};
