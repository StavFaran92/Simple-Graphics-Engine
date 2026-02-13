#pragma once

#include "sge.h"

class UIEntitySelectionLayer : public EventLayer
{
public:
	UIEntitySelectionLayer() : EventLayer("UIEntitySelectionLayer")
	{
	}

	bool handleEvent(SDL_Event e) override;
};