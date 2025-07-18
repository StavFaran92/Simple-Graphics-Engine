#pragma once

#include "core/Core.h"

class EngineAPI ISelectable
{
	virtual void onSelected() = 0;
	virtual void select() = 0;
};
