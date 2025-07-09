#include "TerrainMaterial.h"

#include "Logger.h"
#include <GL\glew.h>

#include "Engine.h"
#include "Context.h"
#include "CommonTextures.h"

const std::vector<Resource<Texture>>& TerrainMaterial::getAllTextures() const
{
	return m_textures;
}
