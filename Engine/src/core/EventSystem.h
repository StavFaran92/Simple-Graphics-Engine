#pragma once
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <memory>
#include <queue>
#include <functional>
#include <string>

#include "core/Core.h"

#include "core/EventLayer.h"

#include "core/Event.h"

class EngineAPI EventSystem
{
public:
	EventSystem() = default;
	EventSystem(const EventSystem&) = delete;
	EventSystem& operator=(const EventSystem&) = delete;

	void subscribe(EventHandler handler, EventType eventType, Subscriber* s);
	void subscribe(EventHandler handler, EventType eventType, Callback c);
	void unsubscribe(EventHandler handler, EventType eventType);
	
	EventHandler bindToLayer(const std::string& layerName);
	template<typename T>
	void pushEvent(T e)
	{
		static_assert(std::is_base_of<Event, T>::value, "T must derive from Event");
		m_customEvents.push(std::make_unique<T>(std::move(e)));
	}

	void dispatch(const Event& e);
	void pushLayer(std::shared_ptr<EventLayer> layer);
	void popLayer();

	std::shared_ptr<EventLayer> getLayer(const std::string& layerName);
	std::shared_ptr<EventLayer> getLayer(EventHandler handler);

private:
	friend class Engine;
	std::unique_ptr<Event> pollEvent();

private:
	inline static uint64_t s_currentSubscriber = 0;

	std::vector<std::shared_ptr<EventLayer>> m_layers;

	std::unordered_map<EventHandler, std::string> m_handlerLayerMap;

	std::queue<std::unique_ptr<Event>> m_customEvents;
};
