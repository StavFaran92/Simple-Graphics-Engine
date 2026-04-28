#pragma once

#include "sge.h"

#include "DialogBase.h"

class TextureSelectDialog : public DialogBase
{
public:
	TextureSelectDialog();
	void appearContent() override;
	void drawContent() override;
	bool acceptContent() override;
	void cancelContent() override;

	std::function<void(UUID)> onAcceptCB;
private:
	int m_selectedTextureIndex = -1;
	TextureResourceRef displayTexture;
};
