#pragma once

#include "sge.h"

class UIEventLayer : public EventLayer
{
public:
	UIEventLayer() : EventLayer("UILayer")
	{
	}

	bool handleEvent(const Event& e) override;
};