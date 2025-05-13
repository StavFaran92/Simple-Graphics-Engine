#include "Mouse.h"

#include "Engine.h"
#include "EventSystem.h"

const Mouse::MouseState& Mouse::getMouseState()
{
	int x, y;
	Uint32 buttons;

	SDL_PumpEvents();  // make sure we have the latest mouse state.

	buttons = SDL_GetMouseState(&x, &y);

	m_state.x = x;
	m_state.y = y;
	m_state.lmb = buttons & SDL_BUTTON_LMASK;
	m_state.rmb = buttons & SDL_BUTTON_RMASK;
	m_state.mmb = buttons & SDL_BUTTON_MMASK;

	return m_state;
}

void Mouse::getMousePosition(int& x, int& y)
{
	const MouseState& state = getMouseState();

	x = state.x;
	y = state.y;
}

bool Mouse::getButtonPressed(MouseButton button) 
{
	bool result = false;

	const MouseState& state = getMouseState();

	switch (button)
	{
	case MouseButton::LeftMousebutton:
		result = state.lmb;
		break;
	case MouseButton::RightMousebutton:
		result = state.rmb;
		break;
	case MouseButton::MiddleMousebutton:
		result = state.mmb;
		break;
	default:
		break;
	}
	
	return result;
}

void Mouse::onMousePressed(MouseButton code, std::function<void(SDL_Event e)> callback, std::shared_ptr<EventSystem> eventSystem) const
{
	eventSystem->addEventListener(SDL_EventType::SDL_MOUSEBUTTONDOWN, [=](SDL_Event e)
	{
		if (e.button.button == mouseButtonToSDLCode(code))
		{
			callback(e);
		}
	});
}

void Mouse::onMouseReleased(MouseButton code, std::function<void(SDL_Event e)> callback, std::shared_ptr<EventSystem> eventSystem) const
{
	eventSystem->addEventListener(SDL_EventType::SDL_MOUSEBUTTONUP, [=](SDL_Event e)
	{
		if (e.button.button == mouseButtonToSDLCode(code))
		{
			callback(e);
		}
	});
}

void Mouse::onEvent(SDL_Event e)
{
}

int Mouse::mouseButtonToSDLCode(MouseButton button) const
{
	int result = -1;

	switch (button)
	{
	case MouseButton::LeftMousebutton:
		result = SDL_BUTTON_LEFT;
		break;
	case MouseButton::RightMousebutton:
		result = SDL_BUTTON_RIGHT;
		break;
	case MouseButton::MiddleMousebutton:
		result = SDL_BUTTON_MIDDLE;
		break;
	default:
		break;
	}

	return result;
}
