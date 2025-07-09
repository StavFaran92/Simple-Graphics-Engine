#pragma once

class Scene;

#include "Resource.h"

class EquirectangularToCubemapConverter
{
public:
	static Resource<Texture> fromEquirectangularToCubemap(Resource<Texture> equirectangularTexture);
	static Resource<Texture> fromCubemapToEquirectangular(Resource<Texture> cubemapTexture);
};