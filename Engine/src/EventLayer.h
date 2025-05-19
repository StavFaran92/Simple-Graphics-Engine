#pragma once

#include "SDL.h"
#include <functional>
#include <string>
#include <cstdint>

#include "Subscriber.h"

using EventHandler = uint64_t;

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

	virtual void subscribe(SDL_EventType eventType, Subscriber* s) = 0;

	virtual void unsubscribe(SDL_EventType eventType, Subscriber* s) = 0;

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
};