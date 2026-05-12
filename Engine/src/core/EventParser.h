#pragma once

#include "SDL2/SDL_events.h"

class Event;



class EventParser
{
public:
	static Event* parseSDLEvent(SDL_Event e);
};