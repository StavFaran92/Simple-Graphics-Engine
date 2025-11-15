#pragma once

#include "DialogBase.h"
#include "Widgets.h"

class AnimationImportDialog : public DialogBase
{
public:
	AnimationImportDialog();
	// Inherited via DialogBase
	void appearContent() override;
	void drawContent() override;
	bool acceptContent() override;
	void cancelContent() override;

	UniqueNameWidget uniqueName{ "Name" };
	FilepathWidget filepath{ "##Filepath", Constants::g_animationSupportedFormats, 1 };
};