#pragma once

#include "DialogBase.h"

class TextureSelectDialog : public DialogBase
{
public:
	TextureSelectDialog();
	void appearContent() override;
	void drawContent() override;
	bool acceptContent() override;
	void cancelContent() override;

private:
	int m_selectedTextureIndex = -1;
};
