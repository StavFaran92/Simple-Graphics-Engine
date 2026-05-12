#pragma once

#include "EditorTool.h"
#include "sge.h"

class FoliagePaintTool : public EditorTool
{
	// Inherited via EditorTool
	void update(ImVec2 viewportPos, ImVec2 viewportSize) override;
	bool onEvent(const Event& e) override;
	const char* name() const override;

private:
	float m_brushRadius = 3.f;
	float m_brushStrength = .01f;

	RayHit m_currentResult;
};