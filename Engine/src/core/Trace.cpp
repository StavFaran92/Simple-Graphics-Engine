#include "core/Trace.h"

#include "core/TraceLogger.h"
#include "core/Engine.h"
#include "core/System.h"

void Trace::assetDependency(const UUID& from, const UUID& to)
{
	auto frame = Engine::get()->getSubSystem<System>()->getFrameCount();

	nlohmann::json j = {
		{"frame", frame},
		{"system", "dependency"},
		{"type", "edge"},
		{"from", from},
		{"to", to}
	};
	TraceLogger::writeJsonLine(j);
}
