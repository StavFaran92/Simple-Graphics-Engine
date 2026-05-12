#pragma once

#include <memory>
#include "SDL2/SDL_events.h"
#include "core/Event.h"

class EventParser
{
public:
	static std::unique_ptr<Event> parseSDLEvent(SDL_Event e);
};
