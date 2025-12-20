#pragma once

#include "EditorTool.h"

class TransformTool : public EditorTool
{
	// Inherited via EditorTool
	void update(ImVec2 viewportPos, ImVec2 viewportSize) override;
	const char* name() const override;
};