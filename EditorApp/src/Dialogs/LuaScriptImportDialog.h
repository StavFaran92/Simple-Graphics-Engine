#pragma once

#include "DialogBase.h"
#include "Widgets.h"

#include "Widgets/UniqueNameWidget.h"
#include "Widgets/FilepathWidget.h"

class LuaScriptImportDialog : public DialogBase
{
public:
	LuaScriptImportDialog();
	// Inherited via DialogBase
	void appearContent() override;
	void drawContent() override;
	bool acceptContent() override;
	void cancelContent() override;

	UniqueNameWidget uniqueName{ "Name" };
	FilepathWidget filepath{ "##Filepath", Constants::g_luaScriptSupportedFormats, 1 };
};