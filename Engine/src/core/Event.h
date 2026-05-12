#pragma once

#include <string>

#include "core/Core.h"


enum class EventType {
    BaseEvent,
    KeyPressed,
    KeyReleased,
    MouseMoved,
    MouseButtonPressed,
    MouseButtonReleased,
    MouseWheel,
    WindowResized,
    WindowClosed,
    QuitApp
};

struct EngineAPI Event
{
	std::string getName() const;
    virtual EventType type() const { return EventType::BaseEvent;  };
    bool handled = false;
};