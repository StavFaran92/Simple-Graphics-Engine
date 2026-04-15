#pragma once

#include <string>

#include "memory/AssetHandle.h"
#include "memory/Assets.h"
#include "core/Engine.h"

const std::string SGE_TEXTURE_WHITE = "SGE_TEXTURE_WHITE";
const std::string SGE_TEXTURE_BLACK = "SGE_TEXTURE_BLACK";

const std::string SGE_MESH_GRID = "SGE_MESH_GRID";
const std::string SGE_MESH_BOX = "SGE_MESH_BOX";
const std::string SGE_MESH_QUAD = "SGE_MESH_QUAD";
const std::string SGE_MESH_SPHERE = "SGE_MESH_SPHERE";
const std::string SGE_MESH_CAMERA = "SGE_MESH_CAMERA";

const std::string SGE_MATERIAL_DEFAULT = "SGE_MATERIAL_DEFAULT";
const std::string SGE_MATERIAL_TERRAIN_DEFAULT = "SGE_MATERIAL_TERRAIN_DEFAULT";

//const std::string SGE_SHADER_DEFFERED_PBR_GEOM = "SGE_SHADER_DEFFERED_PBR_GEOM";
//const std::string SGE_SHADER_DEFFERED_PBR_LIGHT = "SGE_SHADER_DEFFERED_PBR_LIGHT";
//const std::string SGE_SHADER_FORWARD_PBR = "SGE_SHADER_FORWARD_PBR";
//const std::string SGE_SHADER_TERRAIN = "SGE_SHADER_TERRAIN";
//const std::string SGE_SHADER_DEBUG_DATA = "SGE_SHADER_DEBUG_DATA";

// Registry for engine-shipped, user-selectable content.
// Entries have stable UUIDs, are serialized when referenced,
// and are exposed through the asset browser.
class BuiltInAssets
{
public:
	template<typename T>
	static AssetHandle<T> get(const UUID& uuid)
	{
		if (!Engine::get()->getSubSystem<Assets>()->hasAsset(uuid))
		{
			logWarning("Could not find asset {}", uuid.str());
			return {};
		}
		return Engine::get()->getSubSystem<Assets>()->getAsset(uuid).data().as<T>();
	}

	template<typename T>
	static AssetHandle<T> getByName(const std::string& name)
	{
		return Engine::get()->getSubSystem<Assets>()->getAssetFromName(name).as<T>();
	}

	template<typename T>
	static AssetHandle<T> getByPath(const std::string& path)
	{
		return Engine::get()->getSubSystem<Assets>()->getAssetFromPath(path).as<T>();
	}
};