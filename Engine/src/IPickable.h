#pragma once

#include "Core.h"

class EngineAPI IPickable
{
	virtual void onPicked() = 0;
	virtual void pick() = 0;
};
