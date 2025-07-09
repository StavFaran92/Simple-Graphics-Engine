#pragma once

#include <stdio.h>

#include "SDL2/SDL_syswm.h"
#include "SDL2/SDL.h"
#include "Core.h"

#include "Configurations.h"

class EngineAPI Window
{
public:

	Window();

	int init();

	inline int getWidth() { return m_width; }
	inline int getHeight() { return m_height; }

	void close();

	void SwapBuffer();
	void update();
	void lockMouse() { m_isMouseLocked = true; }
	void unlockMouse() { m_isMouseLocked = false; }

	SDL_Window* GetWindow() { return m_mainWindow; }
	SDL_GLContext GetContext() { return m_glContext; }

	void* GetNativeWindow();

	~Window();
private:
	SDL_Window* m_mainWindow = nullptr;
	SDL_GLContext m_glContext = nullptr;

	int m_width = 0;
	int m_height = 0;

	int m_halfWidth = 0;
	int m_halfHeight = 0;

	int m_mousePreviousX = 0;
	int m_mousePreviousY = 0;
	int m_mouseX = 0;
	int m_mouseY = 0;

	bool m_isMouseLocked = false;

	bool m_mouseFirstMoved = false;
};
