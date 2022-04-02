#pragma once

#include "SDL.h"

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"



class ImguiHandler
{
public:
	bool init(SDL_Window* window, const SDL_GLContext& context);
	void ProccessEvents(SDL_Event& e);
	void Render();
	bool close();
};

