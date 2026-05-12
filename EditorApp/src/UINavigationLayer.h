#pragma once

#include "sge.h"

class UINavigationLayer : public EventLayer
{
public:
	UINavigationLayer() : EventLayer("UINavigationLayer")
	{
	}

	bool handleEvent(const Event& e) override;
};