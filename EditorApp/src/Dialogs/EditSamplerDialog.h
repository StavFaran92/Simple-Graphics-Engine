#pragma once

#include "DialogBase.h"
#include "Widgets.h"
#include "sge.h"

#include <functional>
#include <memory>

class EditSamplerDialog : public DialogBase
{
public:
	EditSamplerDialog();
	void appearContent() override;
	void drawContent() override;
	bool acceptContent() override;
	void cancelContent() override;

public:
	std::shared_ptr<TextureSamplerAsset> sampler;
	std::function<void(UUID)> onAcceptCB;

private:
	std::shared_ptr<TextureSamplerAsset> m_previousSampler;
};
