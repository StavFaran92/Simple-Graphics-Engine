#include "core/Trace.h"

#include "core/TraceLogger.h"
#include "core/Engine.h"
#include "core/System.h"
#include "systems/TimeManager.h"
#include "memory/Assets.h"

void Trace::addSceneAssetMonitor(const SceneResourceRef& scene, const UUID& uid)
{
	auto frame = Engine::get()->getSubSystem<System>()->getFrameCount();
	auto globalTime = Engine::get()->getSubSystem<TimeManager>()->getCurrentTime(TimeManager::Duration::MicroSeconds);

	nlohmann::json j = {
		{"frame", frame},
		{"global_time", globalTime},
		{"system", "scene_monitor"},
		{"type", "add_dependency"},
		{"scene", scene.getUID()},
		{"asset_id", uid}
	};
	TraceLogger::writeJsonLine(j);
}

void Trace::removeSceneAssetMonitor(const SceneResourceRef& scene, const UUID& uid)
{
	auto frame = Engine::get()->getSubSystem<System>()->getFrameCount();
	auto globalTime = Engine::get()->getSubSystem<TimeManager>()->getCurrentTime(TimeManager::Duration::MicroSeconds);

	nlohmann::json j = {
		{"frame", frame},
		{"global_time", globalTime},
		{"system", "scene_monitor"},
		{"type", "remove_dependency"},
		{"scene", scene.getUID()},
		{"asset_id", uid}
	};
	TraceLogger::writeJsonLine(j);
}

void Trace::destroyResource(ResourceID id, const std::string& type)
{
	auto frame = Engine::get()->getSubSystem<System>()->getFrameCount();
	auto globalTime = Engine::get()->getSubSystem<TimeManager>()->getCurrentTime(TimeManager::Duration::MicroSeconds);

	nlohmann::json j = {
		{"frame", frame},
		{"global_time", globalTime},
		{"system", "resource_view"},
		{"type", "destroy_resource"},
		{"id", id},
		{"resource_type", type}
	};
	TraceLogger::writeJsonLine(j);
}

void Trace::createResource(ResourceID id, const std::string& type)
{
	auto frame = Engine::get()->getSubSystem<System>()->getFrameCount();
	auto globalTime = Engine::get()->getSubSystem<TimeManager>()->getCurrentTime(TimeManager::Duration::MicroSeconds);

	nlohmann::json j = {
		{"frame", frame},
		{"global_time", globalTime},
		{"system", "resource_view"},
		{"type", "add_resource"},
		{"id", id},
		{"resource_type", type}
	};
	TraceLogger::writeJsonLine(j);
}

void Trace::createAsset(UUID assetID, const AssetRecord& record)
{
	auto frame = Engine::get()->getSubSystem<System>()->getFrameCount();
	auto globalTime = Engine::get()->getSubSystem<TimeManager>()->getCurrentTime(TimeManager::Duration::MicroSeconds);

	nlohmann::json j = {
		{"frame", frame},
		{"global_time", globalTime},
		{"system", "asset_view"},
		{"type", "add_asset"},
		{"id", assetID},
		{"name", record.name},
		{"asset_type", getAssetTypeAsStr(record.aType)},
		{"owner", record.isEngineOwned ? "Engine" : "User"},
		{"source", record.sourcePath},
		{"ext", record.ext},
		{"path", record.getAbsolutePath()}
	};
	TraceLogger::writeJsonLine(j);
}

void Trace::bindResourceToAsset(UUID assetID, ResourceID resourceID)
{
	auto frame = Engine::get()->getSubSystem<System>()->getFrameCount();
	auto globalTime = Engine::get()->getSubSystem<TimeManager>()->getCurrentTime(TimeManager::Duration::MicroSeconds);

	nlohmann::json j = {
		{"frame", frame},
		{"global_time", globalTime},
		{"system", "asset_view"},
		{"type", "bind_resource"},
		{"asset_id", assetID},
		{"resource_id", resourceID}
	};
	TraceLogger::writeJsonLine(j);
}

void Trace::setProjectRootFolder(const std::string& projectFolder)
{
	auto frame = Engine::get()->getSubSystem<System>()->getFrameCount();
	auto globalTime = Engine::get()->getSubSystem<TimeManager>()->getCurrentTime(TimeManager::Duration::MicroSeconds);

	nlohmann::json j = {
		{"frame", frame},
		{"global_time", globalTime},
		{"system", "asset_view"},
		{"type", "project_folder"},
		{"folder", projectFolder}
	};
	TraceLogger::writeJsonLine(j);
}