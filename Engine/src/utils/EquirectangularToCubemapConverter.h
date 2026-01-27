#pragma once

class Scene;
class Texture;

#include "memory/ResourceWrapper.h"

class EquirectangularToCubemapConverter
{
public:
	static ResourceWrapper<Texture> fromEquirectangularToCubemap(ResourceWrapper<Texture> equirectangularTexture);
	static ResourceWrapper<Texture> fromCubemapToEquirectangular(ResourceWrapper<Texture> cubemapTexture);
};