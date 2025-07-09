#pragma once
#include "core/Core.h"
#include "SDL2/SDL.h"
#include "Subscriber.h"

#include <functional>

#include "core/EventLayer.h"


class EngineAPI Mouse
{
public:
	Mouse();
	enum class MouseButton
	{
		LeftMousebutton,
		RightMousebutton,
		MiddleMousebutton,
	};
	struct MouseState
	{
		int x = 0;
		int y = 0;
		bool lmb = false;
		bool rmb = false;
		bool mmb = false;
	};

	const MouseState& getMouseState();
	void getMousePosition(int& x, int& y);
	bool getButtonPressed(MouseButton button);
	void onMousePressed(EventHandler handler, MouseButton code, std::function<void(SDL_Event e)> callback) const;
	void onMouseReleased(EventHandler handler, MouseButton code, std::function<void(SDL_Event e)> callback) const;


private:
	int mouseButtonToSDLCode(MouseButton button) const;
	MouseState m_state;

	//uint64_t m_eventHandler;
};
