#include "render/TerrainMaterial.h"

#include "systems/Logger.h"
#include <GL\glew.h>

#include "core/Engine.h"
#include "Context.h"
#include "CommonTextures.h"

const std::vector<Resource<Texture>>& TerrainMaterial::getAllTextures() const
{
	return m_textures;
}
