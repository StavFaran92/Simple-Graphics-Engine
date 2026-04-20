#include "core/Trace.h"

#include "core/TraceLogger.h"
#include "core/Engine.h"
#include "core/System.h"

void Trace::addSceneAssetMonitor(const SceneResourceRef& scene, const UUID& uid)
{
	auto frame = Engine::get()->getSubSystem<System>()->getFrameCount();

	nlohmann::json j = {
		{"frame", frame},
		{"system", "scene_monitor"},
		{"type", "add_dependency"},
		{"scene", scene.getUID()},
		{"to", uid}
	};
	TraceLogger::writeJsonLine(j);
}

void Trace::removeSceneAssetMonitor(const SceneResourceRef& scene, const UUID& uid)
{
	auto frame = Engine::get()->getSubSystem<System>()->getFrameCount();

	nlohmann::json j = {
		{"frame", frame},
		{"system", "scene_monitor"},
		{"type", "remove_dependency"},
		{"scene", scene.getUID()},
		{"to", uid}
	};
	TraceLogger::writeJsonLine(j);
}
