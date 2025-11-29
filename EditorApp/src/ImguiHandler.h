#pragma once

#include "SDL2/SDL.h"
#include "sge.h"

#include <vector>

class GuiMenu;

class ImguiHandler : public GUILayer
{
public:
	bool init() override;
	void render()override;
	void close() override;

	void addGUI(GuiMenu* menu);
	void removeGUI(GuiMenu* menu);

	void* getCurrentContext() const;

private:
	std::vector<GuiMenu*> m_guiList;

	void* m_imguiContext = nullptr;
};

