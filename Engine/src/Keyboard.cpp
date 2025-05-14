#include "Keyboard.h"
#include "Engine.h"
#include "EventSystem.h"
#include "Logger.h"

Keyboard::Keyboard() 
{
	m_keyboardState = SDL_GetKeyboardState(&m_length);

	m_eventHandler = Engine::get()->getEventSystem()->bindToLayer("GameLayer"); // TODO fix
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

void Keyboard::onKeyPressed(SDL_Scancode code, std::function<void(SDL_Event e)> callback) const
{
	if (code < 0 || code > m_length)
	{
		logError("Invalid key specified : " + code);
		return;
	}

	Engine::get()->getEventSystem()->subscribe(m_eventHandler, SDL_EventType::SDL_KEYDOWN, [=](SDL_Event e)
	{
		if (e.key.keysym.scancode == code)
		{
			callback(e);
		}
	});
}

void Keyboard::onKeyReleased(SDL_Scancode code, std::function<void(SDL_Event e)> callback) const
{
	if (code < 0 || code > m_length)
	{
		logError("Invalid key specified : " + code);
		return;
	}

	Engine::get()->getEventSystem()->subscribe(m_eventHandler, SDL_EventType::SDL_KEYUP, [=](SDL_Event e)
	{
		if (e.key.keysym.scancode == code)
		{
			callback(e);
		}
	});
}
