#include "core/Trace.h"

#include "core/TraceLogger.h"

void Trace::dependencyEdge(int frame, const std::string& from, const std::string& to)
{
	nlohmann::json j = {
		{"frame", frame},
		{"system", "dependency"},
		{"type", "edge"},
		{"from", from},
		{"to", to}
	};
	TraceLogger::writeJsonLine(j);
}
