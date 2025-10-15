#pragma once

#include <string>

#include "memory/AssetWrapper.h"
#include "memory/Assets.h"

const UUID SGE_TEXTURE_WHITE = "SGE_TEXTURE_WHITE";
const UUID SGE_TEXTURE_BLACK = "SGE_TEXTURE_BLACK";

const UUID SGE_MESH_GRID = "SGE_MESH_GRID";
const UUID SGE_MESH_BOX = "SGE_MESH_BOX";
const UUID SGE_MESH_QUAD = "SGE_MESH_QUAD";
const UUID SGE_MESH_SPHERE = "SGE_MESH_SPHERE";
const UUID SGE_MESH_CAMERA = "SGE_MESH_CAMERA";

class BuiltInAssets
{
public:
	template<typename T>
	static ResourceWrapper<T> get(const UUID& name)
	{
		if (!Engine::get()->getSubSystem<Assets>()->hasAsset(name))
		{
			logWarning("Could not find asset {}", name);
			return {};
		}
		return Engine::get()->getSubSystem<Assets>()->getAsset(name).data.as<T>();
	}
};