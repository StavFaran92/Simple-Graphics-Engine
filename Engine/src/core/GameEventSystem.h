#pragma once

#include "core/Core.h"
#include "core/EventSystem.h"
#include "systems/SubSystem.h"

class Entity;

class EngineAPI GameEventSystem : public SubSystem
{
public:
	GameEventSystem();
	void subscribe(SDL_EventType eventType, Entity e);
	void unsubscribe(SDL_EventType eventType, Entity e);
	void dispatch(SDL_Event event);

private:
	std::unordered_map<SDL_EventType, std::vector<Entity>> m_handlerTable;
};