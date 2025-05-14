#pragma once

#include "EventLayer.h"

class UIEventLayer : public EventLayer
{
	UIEventLayer() : EventLayer("UILayer")
	{
	}

	bool handleEvent(SDL_Event e) override;

	// Inherited via EventLayer
	void subscribe(SDL_EventType eventType, const std::function<void(SDL_Event e)>& callback) override;
};