#pragma once

#include <string>

#include "core/Core.h"

#include <nlohmann/json.hpp>

class EngineAPI Trace
{
public:
	static void dependencyEdge(int frame, const std::string& from, const std::string& to);

	// Add more trace helpers here (same pattern: build json, TraceLogger::instance().log(j))
};
