#pragma once

#include "EventLayer.h"

class UIEventLayer : public EventLayer
{
public:
	UIEventLayer() : EventLayer("UILayer")
	{
	}

	bool handleEvent(SDL_Event e) override;

	// Inherited via EventLayer
	void subscribe(SDL_EventType eventType, Subscriber* s) override;

	// Inherited via EventLayer
	void unsubscribe(SDL_EventType eventType, Subscriber* s) override;

private:
	std::unordered_map<SDL_EventType, std::vector<Subscriber*>> m_listeners;
};