#pragma once

#include "memory/Asset.h"
#include "core/Core.h"

struct PrefabImportSettings {};

class EngineAPI Prefab : public Asset
{
public:
	static Resource<Prefab> import(const std::string& fileLocation, const PrefabImportSettings& settings);
	static Resource<Prefab> load(AssetInfo aInfo);
};