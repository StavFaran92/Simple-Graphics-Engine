#pragma once

#include "DialogBase.h"

class ProjectSettingsDialog : public DialogBase
{
public:
	ProjectSettingsDialog();
	void appearContent() override;
	void drawContent() override;
	bool acceptContent() override;
	void cancelContent() override;
};
