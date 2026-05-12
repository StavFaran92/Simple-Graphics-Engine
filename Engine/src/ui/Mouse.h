#pragma once
#include "core/Core.h"
#include "core/Subscriber.h"

#include <functional>

#include "core/EventLayer.h"
#include "core/MouseEvents.h"
#include "systems/SubSystem.h"

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

	using MouseButtonPressedCallback  = std::function<bool(MouseButtonPressedEvent)>;
	using MouseButtonReleasedCallback = std::function<bool(MouseButtonReleasedEvent)>;
	using MouseMovedCallback          = std::function<bool(MouseMovedEvent)>;

	const MouseState& getMouseState();
	void getMousePosition(int& x, int& y);
	bool getButtonPressed(MouseButton button);
	void onMousePressed(EventHandler handler, MouseButton button, MouseButtonPressedCallback callback) const;
	void onMouseReleased(EventHandler handler, MouseButton button, MouseButtonReleasedCallback callback) const;
	void onMouseMotion(EventHandler handler, MouseMovedCallback callback) const;

private:
	MouseState m_state;
};

class EngineAPI GameMouse : public Mouse, public SubSystem
{
public:
	GameMouse();
	void onMousePressed(MouseButton button, MouseButtonPressedCallback callback) const;
	void onMouseReleased(MouseButton button, MouseButtonReleasedCallback callback) const;
	void onMouseMotion(MouseMovedCallback callback) const;

private:
	EventHandler gameHandler;
};
