#pragma once

#include <string>

#include "core/Core.h"
#include "memory/AssetAliases.h"

class UUID;
struct AssetBuildDescriptor;

class EngineAPI Trace
{
public:
	static void createResource(ResourceID id, const std::string& type);
	static void destroyResource(ResourceID id, const std::string& type);
	static void setProjectRootFolder(const std::string& projectFolder);
	static void createAsset(UUID assetID, const AssetRecord& record);
	static void bindResourceToAsset(UUID assetID, ResourceID resourceID);
	static void updateSceneResourceVersionCache(ResourceID sceneResurceID, UUID assetID, uint64_t assetVersion);
	static void animGraphInit(uint32_t entityID, const std::string& entryState);
	static void animGraphEnterState(uint32_t entityID, const std::string& fromState, const std::string& toState);

	// Add more trace helpers here (same pattern: build json, TraceLogger::instance().log(j))
};
