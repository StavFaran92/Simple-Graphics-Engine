#pragma once
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <memory>
#include <functional>
#include <string>

#include "SDL2/SDL_events.h"

#include "core/Core.h"

#include "core/EventLayer.h"

class EngineAPI EventSystem
{
public:
	void subscribe(EventHandler handler, SDL_EventType eventType, Subscriber* s);
	void subscribe(EventHandler handler, SDL_EventType eventType, Callback c);
	void unsubscribe(EventHandler handler, SDL_EventType eventType);
	
	EventHandler bindToLayer(const std::string& layerName);
	void pushEvent(SDL_Event e);
	void dispatch(SDL_Event e);
	void pushLayer(std::shared_ptr<EventLayer> layer);
	void popLayer();

	std::shared_ptr<EventLayer> getLayer(const std::string& layerName);
	std::shared_ptr<EventLayer> getLayer(EventHandler handler);

private:
	friend class Engine;

	
	
private:
	inline static uint64_t s_currentSubscriber = 0;

	std::vector<std::shared_ptr<EventLayer>> m_layers;

	std::unordered_map<EventHandler, std::string> m_handlerLayerMap;
};
