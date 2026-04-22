#pragma once

#include <string>

#include "core/Core.h"
#include "memory/AssetAliases.h"

class UUID;

class EngineAPI Trace
{
public:
	static void addSceneAssetMonitor(const SceneResourceRef& scene, const UUID& asset);
	static void removeSceneAssetMonitor(const SceneResourceRef& scene, const UUID& asset);
	static void createResource(ResourceID id, const std::string& type);
	static void destroyResource(ResourceID id, const std::string& type);

	// Add more trace helpers here (same pattern: build json, TraceLogger::instance().log(j))
};
