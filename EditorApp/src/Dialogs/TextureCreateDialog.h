#pragma once

#include "DialogBase.h"
#include "Widgets/UniqueNameWidget.h"
#include "Widgets/TextureDataWidget.h"

class TextureCreateDialog : public DialogBase
{
public:
	TextureCreateDialog();
	void appearContent() override;
	void drawContent() override;
	bool acceptContent() override;
	void cancelContent() override;

private:
	UniqueNameWidget m_uniqueName{ "Name" };
	TextureDataWidget m_textureDataWidget;
	int m_width = 512;
	int m_height = 512;
};
