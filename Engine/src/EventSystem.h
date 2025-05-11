#pragma once
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <memory>
#include <functional>

#include "SDL2/SDL_events.h"

#include "Core.h"

class EngineAPI EventSystem
{
public:
	class Handler
	{
		Handler(EventSystem* eSystem, uint64_t listenerID) : m_eventSystem(eSystem), m_listenerID(listenerID) {}

		~Handler()
		{
			m_eventSystem->removeEventListener(m_listenerID);
		}

	private:
		EventSystem* m_eventSystem;
		const uint64_t m_listenerID;
	};

	struct Callback
	{
		bool isValid = true;
		std::function<void(SDL_Event e)> func;
	};

	uint64_t addEventListener(SDL_EventType eventType, const std::function<void(SDL_Event e)>& callback);
	void addEventListener(SDL_EventType eventType, const std::function<void(SDL_Event e)>& callback, std::shared_ptr<Handler>& handler);
	void removeEventListener(uint64_t listenerID);
	void pushEvent(SDL_Event e);
	void dispatch(SDL_Event e);
private:
	friend class Engine;
	
private:
	std::unordered_map<SDL_EventType, std::unordered_set<uint64_t>> m_listeners;

	std::unordered_map < uint64_t, Callback > m_callbacks;

	inline static uint64_t s_listeners = 0;
};
