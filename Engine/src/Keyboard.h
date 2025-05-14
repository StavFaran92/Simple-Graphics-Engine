#pragma once

#include <iostream>
#include <functional>
#include "Core.h"

#include "SDL2/SDL.h"

class EngineAPI Keyboard
{
public:
	Keyboard();
	int getKeyState(SDL_Scancode code) const;
	void onKeyPressed(SDL_Scancode code, std::function<void(SDL_Event e)> callback) const;
	void onKeyReleased(SDL_Scancode code, std::function<void(SDL_Event e)> callback) const;

private:
	const Uint8* m_keyboardState;
	int m_length;

	uint64_t m_eventHandler;
};
