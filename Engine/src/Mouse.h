#pragma once
#include "Core.h"
#include "SDL2/SDL.h"
#include "Subscriber.h"

#include <functional>


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
	//void onMousePressed(MouseButton code, std::function<void(SDL_Event e)> callback, EventHandler handler, Subscriber* s) const;
	//void onMouseReleased(MouseButton code, std::function<void(SDL_Event e)> callback, EventHandler handler, Subscriber* s) const;


private:
	int mouseButtonToSDLCode(MouseButton button) const;
	MouseState m_state;

	//uint64_t m_eventHandler;
};
