#include "EventSystem.h"

#include "Logger.h"

void EventSystem::subscribe(SDL_EventType eventType, const std::function<void(SDL_Event e)>& callback, entity_id id)
{
	// for iteration u beed:

	// for each layer 
		// get entities that listen on event
			// dispatch to entities

	auto iter = m_entityLayerMap.find(id);
	if (iter == m_entityLayerMap.end())
	{
		logWarning("Could not locate layer for the specified entity, did you forget to register entity to layer?");
		return;
	}

	iter->second->subscribe(eventType, callback);
}

void EventSystem::registerToLayer(entity_id id, std::shared_ptr<EventLayer> layer)
{
	assert(layer);

	m_entityLayerMap[id] = layer;
}

//uint64_t EventSystem::subscribe(SDL_EventType eventType, const std::function<void(SDL_Event e)>& callback)
//{
//	if (m_listeners.find(eventType) == m_listeners.end())
//	{
//		m_listeners[eventType] = {};
//	}
//
//	uint64_t listenterID = ++s_listeners;
//
//	m_listeners[eventType].insert(listenterID);
//
//	m_callbacks[listenterID] = { true, callback };
//
//	return listenterID;
//}

//void EventSystem::subscribe(SDL_EventType eventType, const std::function<void(SDL_Event e)>& callback, std::shared_ptr<Handler>& handler)
//{
//	subscribe(eventType, callback);
//}
//
//void EventSystem::removeEventListener(uint64_t listenerID)
//{
//	auto iter = m_callbacks.find(listenerID);
//	if (iter == m_callbacks.end()) 
//		return;
//
//	iter->second.isValid = false;
//}

void EventSystem::pushEvent(SDL_Event e)
{
	SDL_PushEvent(&e);
}

void EventSystem::dispatch(SDL_Event e)
{
	auto iter = m_layers.rbegin();
	while(iter != m_layers.rend())
	{
		if ((*iter)->handleEvent(e))
		{
			break; // Event was consumed
		}
	}

	//auto& listeners = m_listeners[static_cast<SDL_EventType>(e.type)];
	//auto iter = listeners.begin();
	//while (iter != listeners.end())
	//{
	//	auto& cb = m_callbacks[*iter];
	//	if (!cb.isValid)
	//	{
	//		iter = listeners.erase(iter);
	//		continue;
	//	}

	//	cb.func(e);
	//	iter++;
	//}
}

void EventSystem::pushLayer(std::shared_ptr<EventLayer> layer)
{
	m_layers.push_back(layer);
}

void EventSystem::popLayer()
{
}
