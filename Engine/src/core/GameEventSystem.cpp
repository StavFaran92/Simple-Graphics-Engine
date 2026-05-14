#include "core/GameEventSystem.h"

#include "core/Engine.h"
#include "runtime/Entity.h"
#include "runtime/Context.h"
#include "runtime/Scene.h"
#include "component/ScriptComponent.h"
#include "scripts/ScriptSystem.h"

GameEventSystem::GameEventSystem()
{
	Engine::get()->registerSubSystem<GameEventSystem>(this);
}

void GameEventSystem::subscribe(EventType eventType, Entity e)
{
	m_handlerTable[eventType].push_back(e);
}

void GameEventSystem::unsubscribe(EventType eventType, Entity e)
{
	throw std::exception("not yet impl");
}

void GameEventSystem::dispatch(const Event& event)
{
	//auto& registry = Engine::get()->getContext()->getActiveScene()->getRegistry();
	//registry.getRegistry().view<>
	auto iter = m_handlerTable.find(event.type());
	if (iter != m_handlerTable.end())
	{
		auto& entityVector = iter->second;
		for (Entity& e : entityVector)
		{
			Engine::get()->getSubSystem<ScriptSystem>()->callOnEvent(e, event);
		}
	}
	
}

void GameEventSystem::clean()
{
	m_handlerTable.clear();
}

//void GameEventSystem::unsubscribe(SDL_EventType eventType, Subscriber* s)
//{
//	auto iter = m_handlerTable.find(s);
//	if (iter == m_handlerTable.end())
//	{
//		return;
//	}
//	
//	EventHandler handler = iter->second;
//
//	Engine::get()->getEventSystem()->unsubscribe(handler, eventType);
//}
