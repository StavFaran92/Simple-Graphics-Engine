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
	std::unordered_map<SDL_EventType, std::vector<EventCallback>> m_listeners;

	// Inherited via EventLayer
	void subscribe(SDL_EventType eventType, const EventCallback& callback) override;

	// Inherited via EventLayer
	void unsubscribe(EventHandler handler, SDL_EventType eventType) override;
};