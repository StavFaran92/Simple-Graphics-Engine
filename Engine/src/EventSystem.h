#pragma once
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <memory>
#include <functional>

#include "SDL2/SDL_events.h"

#include "Core.h"

#include "EventLayer.h"

#include "Entity.h"

class EngineAPI EventSystem
{
public:
	struct Callback
	{
		bool isValid = true;
		std::function<void(SDL_Event e)> func;
	};

	void subscribe(SDL_EventType eventType, const std::function<void(SDL_Event e)>& callback, entity_id id);
	void registerToLayer(entity_id id, std::shared_ptr<EventLayer> layer);
	//uint64_t subscribe(SDL_EventType eventType, const std::function<void(SDL_Event e)>& callback);
	//void subscribe(SDL_EventType eventType, const std::function<void(SDL_Event e)>& callback, std::shared_ptr<Handler>& handler);
	//void removeEventListener(uint64_t listenerID);
	void pushEvent(SDL_Event e);
	void dispatch(SDL_Event e);
	void pushLayer(std::shared_ptr<EventLayer> layer);
	void popLayer();

private:
	friend class Engine;
	
private:
	//std::unordered_map<SDL_EventType, std::unordered_set<uint64_t>> m_listeners;

	//std::unordered_map < uint64_t, Callback > m_callbacks;

	//inline static uint64_t s_listeners = 0;

	std::unordered_map<entity_id, bool> m_isHandled;

	std::vector<std::shared_ptr<EventLayer>> m_layers;

	std::unordered_map<entity_id, std::shared_ptr<EventLayer>> m_entityLayerMap;
};
