#pragma once

#include <unordered_set>
#include <memory>

#include "EventLayer.h"

class ScriptableEntity;

class GameLayer : public EventLayer
{
public:
	bool handleEvent(SDL_Event event) override;

	void subscribe(std::shared_ptr<ScriptableEntity> e);
	void unsubscribe(std::shared_ptr<ScriptableEntity> e);

private:
	std::unordered_set<std::shared_ptr<ScriptableEntity>> m_listeners;
};