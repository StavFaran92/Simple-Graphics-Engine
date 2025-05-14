#pragma once

#include "SDL.h"
#include <functional>

class EventLayer
{
public:
	virtual bool handleEvent(SDL_Event e) = 0;

	virtual void subscribe(SDL_EventType eventType, const std::function<void(SDL_Event e)>& callback) = 0;
};