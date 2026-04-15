#pragma once

#include "core/Core.h"
#include "runtime/Entity.h"
#include "systems/SubSystem.h"
#include "texture/Texture.h"
#include "memory/ResourceRef.h"

struct VolumetricCloudsComponent;

class EngineAPI VolumetricCloudsSystem : public SubSystem
{
public:
	VolumetricCloudsSystem();
	static Entity createVolumetricClouds();

	//static void drawWaterBody(const WaterBodyComponent& waterBody);

	void prepareVolumetricCloudsForRender(VolumetricCloudsComponent& clouds);

private:
	TextureResourceRef m_noiseTexture;
};