#pragma once

#include "EditorTool.h"

#include "ImGuizmo.h"

class TransformTool : public EditorTool
{
	// Inherited via EditorTool
	void update(ImVec2 viewportPos, ImVec2 viewportSize) override;
	bool onEvent(SDL_Event e) override;
	const char* name() const override;

private:
	ImGuizmo::OPERATION m_operationMode = ImGuizmo::TRANSLATE;
	ImGuizmo::MODE m_currentGizmoMode = ImGuizmo::LOCAL;
	bool m_useSnap = false;
	float m_snapValues[3] = { 1.0f, 1.0f, 1.0f };

	bool m_toolbarActive = false;
};