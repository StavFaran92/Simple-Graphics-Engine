#pragma once

#include "core/Event.h"

enum MouseButton : int
{
	INVALID          = 0,
	MOUSE_BUTTON_LEFT   = 1,
	MOUSE_BUTTON_MIDDLE = 2,
	MOUSE_BUTTON_RIGHT  = 3,
};

struct MouseButtonPressedEvent : public Event
{
	EventType type() const override { return EventType::MouseButtonPressed; }
	MouseButton button = INVALID;
	int32_t x = 0;
	int32_t y = 0;
	uint8_t clicks = 0;
};

struct MouseButtonReleasedEvent : public Event
{
	EventType type() const override { return EventType::MouseButtonReleased; }
	MouseButton button = INVALID;
	int32_t x = 0;
	int32_t y = 0;
	uint8_t clicks = 0;
};

struct MouseMovedEvent : public Event
{
	EventType type() const override { return EventType::MouseMoved; }
	int32_t x = 0;
	int32_t y = 0;
	int32_t xrel = 0;
	int32_t yrel = 0;
};

struct MouseWheelEvent : public Event
{
	EventType type() const override { return EventType::MouseWheel; }
	int32_t x = 0;
	int32_t y = 0;
};
