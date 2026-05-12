#pragma once

#include "core/Event.h"

struct QuitAppEvent : public Event
{
	EventType type() const override { return EventType::QuitApp; }
};