#pragma once

class Scene;
#include "Resource.h"

class IBL
{
public:
	static Resource<Texture> generateIrradianceMap(Resource<Texture> cubemap, Scene* scene);
	static Resource<Texture> generatePrefilterEnvMap(Resource<Texture> cubemap, Scene* scene);
	static Resource<Texture> generateBRDFIntegrationLUT(Scene* scene);
};
