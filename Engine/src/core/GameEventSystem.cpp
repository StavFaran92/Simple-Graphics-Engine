#include "core/GameEventSystem.h"

#include "core/Engine.h"

void GameEventSystem::subscribe(SDL_EventType eventType, Subscriber* s)
{
	EventHandler handler = Engine::get()->getEventSystem()->bindToLayer("GameLayer");
	m_handlerTable[s] = handler;

	Engine::get()->getEventSystem()->subscribe(handler, eventType, s);
}

void GameEventSystem::unsubscribe(SDL_EventType eventType, Subscriber* s)
{
	auto iter = m_handlerTable.find(s);
	if (iter == m_handlerTable.end())
	{
		return;
	}
	
	EventHandler handler = iter->second;

	Engine::get()->getEventSystem()->unsubscribe(handler, eventType);
}
