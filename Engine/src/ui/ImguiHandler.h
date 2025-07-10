#pragma once

#include "SDL2/SDL.h"
#include "Core.h"

#include <vector>

class GuiMenu;

class EngineAPI ImguiHandler
{
public:
	bool init(SDL_Window* window, const SDL_GLContext& context);
	void proccessEvents(SDL_Event& e);
	void render();
	bool close();

	void addGUI(GuiMenu* menu);
	void removeGUI(GuiMenu* menu);

	void* getCurrentContext() const;

private:
	std::vector<GuiMenu*> m_guiList;

	void* m_imguiContext = nullptr;
};

