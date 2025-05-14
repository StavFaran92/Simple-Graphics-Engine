#pragma once

#include <vector>
#include <memory>

#include "EventLayer.h"

class GameLayer : public EventLayer
{
public:
	GameLayer();
	bool handleEvent(SDL_Event event) override;

private:
	std::unordered_map<SDL_EventType, std::vector<std::function<void(SDL_Event e)>>> m_listeners;

	// Inherited via EventLayer
	void subscribe(SDL_EventType eventType, const std::function<void(SDL_Event e)>& callback) override;
};