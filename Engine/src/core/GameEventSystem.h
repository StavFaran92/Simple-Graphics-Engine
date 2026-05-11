#pragma once

#include "core/Core.h"
#include "core/EventSystem.h"

class EngineAPI GameEventSystem
{
public:
	void subscribe(SDL_EventType eventType, Subscriber* s);
	void unsubscribe(SDL_EventType eventType, Subscriber* s);

private:
	std::unordered_map<Subscriber*, EventHandler> m_handlerTable;
};