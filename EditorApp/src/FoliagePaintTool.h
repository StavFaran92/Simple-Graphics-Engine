#pragma once

#include "EditorTool.h"

struct RayHit {
	bool hit = false;
	float t = 0.0f;
	glm::vec3 position{};
	glm::vec3 normal{};
};

class FoliagePaintTool : public EditorTool
{
	// Inherited via EditorTool
	void update(ImVec2 viewportPos, ImVec2 viewportSize) override;
	bool onEvent(SDL_Event e) override;
	const char* name() const override;

private:
	RayHit m_currentResult;
};