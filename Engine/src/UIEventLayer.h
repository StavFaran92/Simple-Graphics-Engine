#pragma once

#include "EventLayer.h"

class UIEventLayer : public EventLayer
{
	bool handleEvent(SDL_Event e) override;
};