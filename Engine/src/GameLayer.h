#pragma once

#include <unordered_set>
#include <memory>

#include "EventLayer.h"
#include "EventSystem.h"

class ScriptableEntity;

class GameLayer : public EventLayer
{
public:
	GameLayer();
	bool handleEvent(SDL_Event event) override;
	std::shared_ptr<EventSystem> getEventSystem() const;

private:
	std::shared_ptr<EventSystem> m_eventSystem;
};