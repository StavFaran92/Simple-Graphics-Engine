#pragma once

#include "core/Core.h"


enum class EventType {
    BaseEvent,
    KeyPressed, 
    KeyReleased,
    MouseMoved, 
    MouseButtonPressed,
    MouseButtonReleased,
    WindowResized, 
    WindowClosed,
};

class EngineAPI Event
{
public:
	std::string getName() const;
    virtual EventType type() const { return EventType::BaseEvent;  };
    bool handled = false;
};