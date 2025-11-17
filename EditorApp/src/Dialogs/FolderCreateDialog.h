#pragma once

#include "DialogBase.h"
#include "Widgets.h"

#include "Widgets/UniqueNameWidget.h"
#include "Widgets/FilepathWidget.h"

class FolderCreateDialog : public DialogBase
{
public:
	FolderCreateDialog();
	// Inherited via DialogBase
	void appearContent() override;
	void drawContent() override;
	bool acceptContent() override;
	void cancelContent() override;

	UniqueNameWidget m_uniqueName{ "Name" };
};