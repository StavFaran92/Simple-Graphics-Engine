#pragma once

#include "core/Event.h"
#include "ui/KeyCodes.h"

enum KeyState : int
{
	Invalid,
	Pressed,
	Released
};

struct KeyPressedEvent : public Event
{
	EventType type() const override
	{
		return EventType::KeyPressed;
	}
	KeyState state = KeyState::Invalid;
	bool repeat = false;
	KeyCode keysym = SCANCODE_UNKNOWN;
};

struct KeyReleasedEvent : public Event
{
	EventType type() const override
	{
		return EventType::KeyReleased;
	}
	KeyState state = KeyState::Invalid;
	bool repeat = false;
	KeyCode keysym = SCANCODE_UNKNOWN;
};