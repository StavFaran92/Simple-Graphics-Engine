#pragma once

#include "core/Subscriber.h"

class SceneEventObserver : public Subscriber
{
public:
	bool onEvent(SDL_Event e) override;

	
};