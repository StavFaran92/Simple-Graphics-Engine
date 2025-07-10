#pragma once

#include <exception>

#include "Core.h"

class EngineAPI ErrorHandler
{
public:
	static int handle(const std::exception& e);
};