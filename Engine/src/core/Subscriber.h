#pragma once

#include "SDL2/SDL.h"

class Subscriber
{
public:
	virtual void onEvent(SDL_Event e) {};
};