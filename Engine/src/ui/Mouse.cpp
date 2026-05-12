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

void Mouse::onMousePressed(EventHandler handler, MouseButton button, MouseButtonPressedCallback callback) const
{
	Engine::get()->getEventSystem()->subscribe(handler, EventType::MouseButtonPressed, [=](const Event& e)
	{
		auto& mEvent = static_cast<const MouseButtonPressedEvent&>(e);
		if (mEvent.button == button)
			return callback(mEvent);
		return false;
	});
}

void Mouse::onMouseReleased(EventHandler handler, MouseButton button, MouseButtonReleasedCallback callback) const
{
	Engine::get()->getEventSystem()->subscribe(handler, EventType::MouseButtonReleased, [=](const Event& e)
	{
		auto& mEvent = static_cast<const MouseButtonReleasedEvent&>(e);
		if (mEvent.button == button)
			return callback(mEvent);
		return false;
	});
}

void Mouse::onMouseMotion(EventHandler handler, MouseMovedCallback callback) const
{
	Engine::get()->getEventSystem()->subscribe(handler, EventType::MouseMoved, [=](const Event& e)
	{
		auto& mEvent = static_cast<const MouseMovedEvent&>(e);
		return callback(mEvent);
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

void GameMouse::onMousePressed(MouseButton button, MouseButtonPressedCallback callback) const
{
	return Mouse::onMousePressed(gameHandler, button, callback);
}

void GameMouse::onMouseReleased(MouseButton button, MouseButtonReleasedCallback callback) const
{
	return Mouse::onMouseReleased(gameHandler, button, callback);
}

void GameMouse::onMouseMotion(MouseMovedCallback callback) const
{
	return Mouse::onMouseMotion(gameHandler, callback);
}
