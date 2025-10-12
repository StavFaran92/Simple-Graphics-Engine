#pragma once

#include <iostream>
#include <functional>
#include "core/Core.h"

#include "core/EventLayer.h"
#include "ui/KeyCodes.h"

class EngineAPI Keyboard
{
public:

	
	enum KeyState : int
	{
		Invalid,
		Pressed,
		Released
	};
	struct KeyEvent
	{
		KeyState state = KeyState::Invalid;
		bool repeat = false;
		Key keysym = SCANCODE_UNKNOWN;
	};
	using KeyCallback = std::function<void(KeyEvent s)>;

	Keyboard();
	int getKeyState(Key code) const;
	void onKeyPressed(EventHandler handler, Key code, KeyCallback callback) const;
	void onKeyReleased(EventHandler handler, Key code, KeyCallback callback) const;

private:
	const uint8_t* m_keyboardState = nullptr;
	int m_length = 0;
};

class EngineAPI GameKeyboard : public Keyboard
{
public:
	GameKeyboard();

private:
	EventHandler gameHandler;
};
