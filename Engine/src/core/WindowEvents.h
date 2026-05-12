#pragma once

#include "core/Event.h"

struct WindowResizedEvent : public Event
{
	EventType type() const override { return EventType::WindowResized; }
	int width  = 0;
	int height = 0;
};

struct WindowClosedEvent : public Event
{
	EventType type() const override { return EventType::WindowClosed; }
};

