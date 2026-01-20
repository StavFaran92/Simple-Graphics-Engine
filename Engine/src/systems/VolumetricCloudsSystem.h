#pragma once

#include "core/Core.h"
#include "runtime/Entity.h"
#include "systems/SubSystem.h"

struct VolumetricCloudsComponent;

class EngineAPI VolumetricCloudsSystem : public SubSystem
{
public:
	VolumetricCloudsSystem();
	static Entity createVolumetricClouds();

	//static void drawWaterBody(const WaterBodyComponent& waterBody);

	static void prepareVolumetricCloudsForRender(VolumetricCloudsComponent& clouds);

private:
	static ResourceWrapper<Texture> m_noiseTexture;
};