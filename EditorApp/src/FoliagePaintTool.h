#pragma once

#include "EditorTool.h"
#include "sge.h"

class FoliagePaintTool : public EditorTool
{
	// Inherited via EditorTool
	void update(ImVec2 viewportPos, ImVec2 viewportSize) override;
	bool onEvent(SDL_Event e) override;
	const char* name() const override;

private:
	RayHit m_currentResult;
};