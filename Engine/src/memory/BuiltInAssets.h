#pragma once

#include <string>

#include "memory/AssetWrapper.h"
#include "memory/Assets.h"

const std::string SGE_TEXTURE_WHITE = "SGE_TEXTURE_WHITE";
const std::string SGE_TEXTURE_BLACK = "SGE_TEXTURE_BLACK";

const std::string SGE_MESH_GRID = "SGE_MESH_GRID";
const std::string SGE_MESH_BOX = "SGE_MESH_BOX";
const std::string SGE_MESH_QUAD = "SGE_MESH_QUAD";
const std::string SGE_MESH_SPHERE = "SGE_MESH_SPHERE";
const std::string SGE_MESH_CAMERA = "SGE_MESH_CAMERA";

const std::string SGE_MATERIAL_DEFAULT = "SGE_MATERIAL_DEFAULT";

class BuiltInAssets
{
public:
	template<typename T>
	static ResourceWrapper<T> get(const UUID& uuid)
	{
		if (!Engine::get()->getSubSystem<Assets>()->hasAsset(uuid))
		{
			logWarning("Could not find asset {}", uuid.str());
			return {};
		}
		return Engine::get()->getSubSystem<Assets>()->getAsset(uuid).data.as<T>();
	}

	template<typename T>
	static ResourceWrapper<T> getByName(const std::string& name)
	{
		UUID uuid = Engine::get()->getSubSystem<Assets>()->getAssetFromName(name);
		return get<T>(uuid);
	}

	template<typename T>
	static ResourceWrapper<T> getByPath(const std::string& path)
	{
		UUID uuid = Engine::get()->getSubSystem<Assets>()->getAssetFromPath(path);
		return get<T>(uuid);
	}
};