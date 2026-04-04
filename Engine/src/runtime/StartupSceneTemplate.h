#pragma once

#include "memory/AssetHandle.h"
#include "runtime/Scene.h"
#include "core/Core.h"

class EngineAPI StartupSceneTemplate
{
public:
	static AssetHandle<SceneAsset> createStartupScene(const std::string& name);
};