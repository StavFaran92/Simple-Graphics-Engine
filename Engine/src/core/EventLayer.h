#pragma once

#include "SDL.h"
#include <functional>
#include <string>
#include <cstdint>

#include "Subscriber.h"

using EventHandler = uint64_t;

using Callback = std::function<void(SDL_Event e)>;

struct EventCallback
{
	EventCallback(EventHandler handler, std::function<void(SDL_Event e)> func) : handler(handler), func(func) {};

	std::function<void(SDL_Event e)> func;
	EventHandler handler;
};

class EventLayer
{
public:
	EventLayer(const std::string& name)
		: name(name)
	{
	};

	virtual bool handleEvent(SDL_Event e) = 0;

	virtual void subscribe(EventHandler handler, SDL_EventType eventType, const Callback& ec)
	{
		m_listeners[eventType].push_back(EventCallback{ handler, ec});
	}

	virtual void unsubscribe(EventHandler handler, SDL_EventType eventType)
	{
		auto iter = m_listeners.find(eventType);
		if (iter != m_listeners.end())
		{
			std::vector<EventCallback>& eventSubscribers = iter->second;
			for (int i = 0; i < eventSubscribers.size(); i++)
			{
				if (eventSubscribers[i].handler == handler)
				{
					eventSubscribers.erase(eventSubscribers.begin() + i);
					return;
				}
			}
		}
	}

	void setEnabled(bool enabled)
	{
		m_isEnabled = enabled;
	}

	bool isEnabled() const
	{
		return m_isEnabled;
	}

	std::string name;

protected:
	bool m_isEnabled = true;

	std::unordered_map<SDL_EventType, std::vector<EventCallback>> m_listeners;
};