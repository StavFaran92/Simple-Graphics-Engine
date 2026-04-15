#pragma once

#include "memory/AssetRef.h"
#include "runtime/Scene.h"
#include "core/Core.h"

class EngineAPI StartupSceneTemplate
{
public:
	static SceneAssetRef createStartupScene(const std::string& name);
};