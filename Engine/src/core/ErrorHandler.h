#pragma once

#include <exception>

#include "core/Core.h"

class EngineAPI ErrorHandler
{
public:
	static int handle(const std::exception& e);
};