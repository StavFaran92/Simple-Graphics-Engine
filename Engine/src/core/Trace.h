#pragma once

#include <string>

#include "core/Core.h"

class UUID;

class EngineAPI Trace
{
public:
	static void assetDependency(const UUID& from, const UUID& to);

	// Add more trace helpers here (same pattern: build json, TraceLogger::instance().log(j))
};
