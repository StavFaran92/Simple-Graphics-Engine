#include "render/TerrainMaterial.h"

#include "core/Logger.h"
#include <GL\glew.h>

#include "core/Engine.h"
#include "runtime/Context.h"


const std::vector<ResourceWrapper<Texture>>& TerrainMaterial::getAllTextures() const
{
	return m_textures;
}
