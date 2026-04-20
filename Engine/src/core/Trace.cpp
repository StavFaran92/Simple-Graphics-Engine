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

void Trace::createNewResource(ResourceID id, const std::string& type)
{
	auto frame = Engine::get()->getSubSystem<System>()->getFrameCount();

	nlohmann::json j = {
		{"frame", frame},
		{"system", "resource_view"},
		{"type", "add_resource"},
		{"id", id},
		{"resource_type", type}
	};
	TraceLogger::writeJsonLine(j);
}
