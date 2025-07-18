#include "ui/Keyboard.h"
#include "core/Engine.h"
#include "core/EventSystem.h"
#include "core/Logger.h"

Keyboard::Keyboard() 
{
	m_keyboardState = SDL_GetKeyboardState(&m_length);
}

int Keyboard::getKeyState(SDL_Scancode code) const
{
	if (code < 0 || code > m_length)
	{
		logError("Invalid key specified : " + code);
		return false;
	}

	return m_keyboardState[code];
}

void Keyboard::onKeyPressed(EventHandler handler, SDL_Scancode code, std::function<void(SDL_Event e)> callback) const
{
	if (code < 0 || code > m_length)
	{
		logError("Invalid key specified : " + code);
		return;
	}

	Engine::get()->getEventSystem()->subscribe(handler, SDL_EventType::SDL_KEYDOWN, [=](SDL_Event e)
	{
		if (e.key.keysym.scancode == code)
		{
			callback(e);
		}
	});
}

void Keyboard::onKeyReleased(EventHandler handler, SDL_Scancode code, std::function<void(SDL_Event e)> callback) const
{
	if (code < 0 || code > m_length)
	{
		logError("Invalid key specified : " + code);
		return;
	}
	Engine::get()->getEventSystem()->subscribe(handler, SDL_EventType::SDL_KEYUP, [=](SDL_Event e)
	{
		if (e.key.keysym.scancode == code)
		{
			callback(e);
		}
	});

}
