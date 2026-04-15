#pragma once

#include "DialogBase.h"
#include "Widgets.h"

#include "Widgets/UniqueNameWidget.h"
#include "Widgets/FilepathWidget.h"
#include "memory/AssetAliases.h"

class SceneCreateDialog : public DialogBase
{
public:
	SceneCreateDialog();
	// Inherited via DialogBase
	void appearContent() override;
	void drawContent() override;
	bool acceptContent() override;
	void cancelContent() override;

	UniqueNameWidget m_uniqueName{ "Name" };
	SceneResourceRef m_tempScene;
};