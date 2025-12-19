#pragma once

#include "EditorTool.h"

class FoliagePaintTool : public EditorTool
{
	// Inherited via EditorTool
	void update() override;
	const char* name() const override;
};