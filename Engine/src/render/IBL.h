#pragma once

class Scene;
#include "memory/ResourceWrapper.h"
#include "texture/Texture.h"

class IBL
{
public:
	static ResourceWrapper<Texture> generateIrradianceMap(ResourceWrapper<Texture> cubemap, Scene* scene);
	static ResourceWrapper<Texture> generatePrefilterEnvMap(ResourceWrapper<Texture> cubemap, Scene* scene);
	static ResourceWrapper<Texture> generateBRDFIntegrationLUT(Scene* scene);
};
