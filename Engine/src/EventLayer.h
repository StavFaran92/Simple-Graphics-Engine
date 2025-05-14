#pragma once

#include "SDL.h"
#include <functional>
#include <string>

class EventLayer
{
public:
	EventLayer(const std::string& name)
		: name(name)
	{
	};

	virtual bool handleEvent(SDL_Event e) = 0;

	virtual void subscribe(SDL_EventType eventType, const std::function<void(SDL_Event e)>& callback) = 0;

	std::string name;
};