#pragma once

#include "core/Core.h"
#include "core/EventSystem.h"
#include "systems/SubSystem.h"

class Entity;

class EngineAPI GameEventSystem : public SubSystem
{
public:
	GameEventSystem();
	void subscribe(EventType eventType, Entity e);
	void unsubscribe(EventType eventType, Entity e);
	void dispatch(const Event& event);

	void clean();

private:
	std::unordered_map<EventType, std::vector<Entity>> m_handlerTable;
};