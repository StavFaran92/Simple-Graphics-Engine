#pragma once

#include "SDL2/SDL.h"

class Subscriber
{
public:
	virtual bool onEvent(SDL_Event e) { return false; };
};