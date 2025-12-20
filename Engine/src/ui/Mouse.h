#pragma once
#include "core/Core.h"
#include "core/Subscriber.h"

#include <functional>

#include "core/EventLayer.h"
#include "systems/SubSystem.h"

enum MouseButton : int
{
	INVALID = 0,
	MOUSE_BUTTON_LEFT = 1,
	MOUSE_BUTTON_MIDDLE = 2,
	MOUSE_BUTTON_RIGHT = 3,
};

class EngineAPI Mouse
{
public:
	Mouse();
	
	struct MouseState
	{
		int x = 0;
		int y = 0;
		bool lmb = false;
		bool rmb = false;
		bool mmb = false;
	};

	enum MouseEventType
	{
		Invalid,
		Motion, 
		ButtonPressed,
		ButtonReleased
	};
	struct MouseEvent
	{
		MouseEventType type = MouseEventType::Invalid;
		int32_t x = 0;
		int32_t y = 0;

		// Only relevant to Motion
		int32_t xrel = 0;
		int32_t yrel = 0;

		// Only relevant to button
		uint8_t clicks;       /**< 1 for single-click, 2 for double-click, etc. */
		MouseButton button = MouseButton::INVALID;
	};

	using MouseCallback = std::function<bool(MouseEvent s)>;

	const MouseState& getMouseState();
	void getMousePosition(int& x, int& y);
	bool getButtonPressed(MouseButton button);
	void onMousePressed(EventHandler handler, MouseButton code, MouseCallback callback) const;
	void onMouseReleased(EventHandler handler, MouseButton code, MouseCallback callback) const;
	void onMouseMotion(EventHandler handler, MouseButton code, MouseCallback callback) const;


private:
	MouseState m_state;
};

class EngineAPI GameMouse : public Mouse, SubSystem
{
public:
	GameMouse();
	void onMousePressed(MouseButton code, MouseCallback callback) const;
	void onMouseReleased(MouseButton code, MouseCallback callback) const;
	void onMouseMotion(MouseButton code, MouseCallback callback) const;

private:
	EventHandler gameHandler;
};
