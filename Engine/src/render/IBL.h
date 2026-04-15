#pragma once

class Scene;
#include "memory/ResourceRef.h"
#include "texture/Texture.h"

class IBL
{
public:
	static TextureResourceRef generateIrradianceMap(TextureResourceRef cubemap);
	static TextureResourceRef generatePrefilterEnvMap(TextureResourceRef cubemap);
	static TextureResourceRef generateBRDFIntegrationLUT();
};
