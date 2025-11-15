#pragma once

#include "DialogBase.h"
#include "Widgets.h"

class TextureImportDialog : public DialogBase
{
public:
	TextureImportDialog();
	// Inherited via DialogBase
	void appearContent() override;
	void drawContent() override;
	bool acceptContent() override;
	void cancelContent() override;

	UniqueNameWidget uniqueName{ "Name" };
	FilepathWidget filepath{ "##Filepath", Constants::g_textureSupportedFormats, 5 };
	TextureDataWidget textureDataWidget;
};