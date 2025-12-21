#pragma once

#include "sge.h"

class UIEditorToolsEventLayer : public EventLayer
{
public:
	UIEditorToolsEventLayer() : EventLayer("UIEditorToolsEventLayer")
	{
	}

	bool handleEvent(SDL_Event e) override;
};