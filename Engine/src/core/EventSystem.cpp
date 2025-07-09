#include "EventSystem.h"

#include "systems/Logger.h"

void EventSystem::subscribe(EventHandler handler, SDL_EventType eventType, Subscriber* s)
{
	auto& layer = getLayer(handler);

	if (!layer)
	{
		logError("Could not locate layer for the specified handler: " + std::to_string(handler));
		return;
	}

	layer->subscribe(handler, eventType, [=](SDL_Event e) { s->onEvent(e); });
}

void EventSystem::subscribe(EventHandler handler, SDL_EventType eventType, Callback c)
{
	auto& layer = getLayer(handler);

	if (!layer)
	{
		logError("Could not locate layer for the specified handler: " + std::to_string(handler));
		return;
	}

	layer->subscribe(handler, eventType, c);
}

void EventSystem::unsubscribe(EventHandler handler, SDL_EventType eventType)
{
	auto& layer = getLayer(handler);

	if (!layer)
	{
		logError("Could not locate layer for the specified handler: " + std::to_string(handler));
		return;
	}

	layer->unsubscribe(handler, eventType);
}

EventHandler EventSystem::bindToLayer(const std::string& layerName)
{
	std::shared_ptr<EventLayer> layer = getLayer(layerName);

	if (layer)
	{
		++s_currentSubscriber;
		m_handlerLayerMap[s_currentSubscriber] = layerName;
		return s_currentSubscriber;
	}

	logWarning("Invalid layer: " + layerName);

	return 0;
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
		iter++;
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

std::shared_ptr<EventLayer> EventSystem::getLayer(const std::string& layerName)
{
	for (auto& layer : m_layers)
	{
		if (layer->name == layerName)
		{
			return layer;
		}
	}

	return nullptr;
}

std::shared_ptr<EventLayer> EventSystem::getLayer(EventHandler handler)
{
	auto iter = m_handlerLayerMap.find(handler);
	if (iter != m_handlerLayerMap.end())
	{
		return getLayer(iter->second);
	}

	return nullptr;
}
