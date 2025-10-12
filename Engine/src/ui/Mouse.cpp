#include "ui/Mouse.h"

#include "core/Engine.h"
#include "core/EventSystem.h"
#include "SDL2/SDL.h"

Mouse::Mouse()
{
}

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

void Mouse::onMousePressed(EventHandler handler, MouseButton code, MouseCallback callback) const
{
	auto eventSystem = Engine::get()->getEventSystem();

	eventSystem->subscribe(handler, SDL_EventType::SDL_MOUSEBUTTONDOWN, [=](SDL_Event e)
	{
		MouseEvent mEvent;
		mEvent.type = Mouse::MouseEventType::ButtonPressed;
		mEvent.button = static_cast<MouseButton>(e.button.button);
		mEvent.clicks = e.button.clicks;
		mEvent.x = e.button.x;
		mEvent.y = e.button.y;
		callback(mEvent);
	});
}

void Mouse::onMouseReleased(EventHandler handler, MouseButton code, MouseCallback callback) const
{
	auto eventSystem = Engine::get()->getEventSystem();

	eventSystem->subscribe(handler, SDL_EventType::SDL_MOUSEBUTTONUP, [=](SDL_Event e)
	{
		MouseEvent mEvent;
		mEvent.type = Mouse::MouseEventType::ButtonReleased;
		mEvent.button = static_cast<MouseButton>(e.button.button);
		mEvent.clicks = e.button.clicks;
		mEvent.x = e.button.x;
		mEvent.y = e.button.y;
		callback(mEvent);
	});
}

void Mouse::onMouseMotion(EventHandler handler, MouseButton code, MouseCallback callback) const
{
	auto eventSystem = Engine::get()->getEventSystem();

	eventSystem->subscribe(handler, SDL_EventType::SDL_MOUSEMOTION, [=](SDL_Event e)
	{
		MouseEvent mEvent;
		mEvent.type = Mouse::MouseEventType::Motion;
		mEvent.x = e.motion.x;
		mEvent.y = e.motion.y;
		mEvent.xrel = e.motion.xrel;
		mEvent.yrel = e.motion.yrel;
		callback(mEvent);
	});
}

bool Mouse::getButtonPressed(MouseButton button)
{
	bool result = false;

	const MouseState& state = getMouseState();

	switch (button)
	{
	case MouseButton::MOUSE_BUTTON_LEFT:
		result = state.lmb;
		break;
	case MouseButton::MOUSE_BUTTON_RIGHT:
		result = state.rmb;
		break;
	case MouseButton::MOUSE_BUTTON_MIDDLE:
		result = state.mmb;
		break;
	default:
		break;
	}

	return result;
}

GameMouse::GameMouse()
{
	gameHandler = Engine::get()->getEventSystem()->bindToLayer("GameLayer");

	Engine::get()->registerSubSystem<GameMouse>(this);
}

void GameMouse::onMousePressed(MouseButton code, MouseCallback callback) const
{
	return Mouse::onMousePressed(gameHandler, code, callback);
}

void GameMouse::onMouseReleased(MouseButton code, MouseCallback callback) const
{
	return Mouse::onMouseReleased(gameHandler, code, callback);
}

void GameMouse::onMouseMotion(MouseButton code, MouseCallback callback) const
{
	return Mouse::onMouseMotion(gameHandler, code, callback);
}
