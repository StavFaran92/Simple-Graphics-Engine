#pragma once

#include "core/Core.h"

class IProjector
{
public:
	virtual bool init(int windowWidth, int windowHeight) = 0;
	virtual void enableWriting() = 0;
	virtual void disableWriting() = 0;
};

