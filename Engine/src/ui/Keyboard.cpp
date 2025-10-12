#include "ui/Keyboard.h"
#include "core/Engine.h"
#include "core/EventSystem.h"
#include "core/Logger.h"

#include "SDL2/SDL.h"

Keyboard::Keyboard() 
{
	m_keyboardState = SDL_GetKeyboardState(&m_length);
}

int Keyboard::getKeyState(KeyCode code) const
{
	if (code < 0 || code > m_length)
	{
		logError("Invalid key specified : " + code);
		return false;
	}

	return m_keyboardState[code];
}

void Keyboard::onKeyPressed(EventHandler handler, KeyCode code, KeyCallback callback) const
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
			KeyEvent kEvent;
			kEvent.keysym = static_cast<KeyCode>(code);
			kEvent.repeat = e.key.repeat != 0;
			kEvent.state = KeyState::Pressed;
			callback(kEvent);
		}
	});
}

void Keyboard::onKeyReleased(EventHandler handler, KeyCode code, KeyCallback callback) const
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
			KeyEvent kEvent;
			kEvent.keysym = static_cast<KeyCode>(code);
			kEvent.repeat = e.key.repeat != 0;
			kEvent.state = KeyState::Released;
			callback(kEvent);
		}
	});

}

GameKeyboard::GameKeyboard() : Keyboard()
{
	gameHandler = Engine::get()->getEventSystem()->bindToLayer("GameLayer");

	Engine::get()->registerSubSystem<GameKeyboard>(this);
}

void GameKeyboard::onKeyPressed(KeyCode code, KeyCallback callback) const
{
	return onKeyPressed(code, callback);
}
