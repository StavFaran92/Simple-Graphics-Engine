#pragma once

class Scene;
class Texture;

#include "memory/AssetAliases.h"

class EquirectangularToCubemapConverter
{
public:
	static TextureResourceRef fromEquirectangularToCubemap(TextureResourceRef equirectangularTexture);
	static TextureResourceRef fromCubemapToEquirectangular(TextureResourceRef cubemapTexture);
};