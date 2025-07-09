#pragma once

#include <vector>
#include <memory>

#include "EventLayer.h"

class GameLayer : public EventLayer
{
public:
	GameLayer() : EventLayer("GameLayer")
	{
	}

	bool handleEvent(SDL_Event event) override;
};