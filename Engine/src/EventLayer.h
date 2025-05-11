#pragma once

#include "SDL.h"

class EventLayer
{
public:
	virtual bool handleEvent(SDL_Event e) = 0;
};