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

private:
	std::unordered_map<SDL_EventType, std::vector<Subscriber*>> m_listeners;

	// Inherited via EventLayer
	void subscribe(SDL_EventType eventType, Subscriber* s) override;

	// Inherited via EventLayer
	void unsubscribe(SDL_EventType eventType, Subscriber* s) override;
};