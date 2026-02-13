#pragma once

#include "sge.h"

class UINavigationLayer : public EventLayer
{
public:
	UINavigationLayer() : EventLayer("UINavigationLayer")
	{
	}

	bool handleEvent(SDL_Event e) override;
};