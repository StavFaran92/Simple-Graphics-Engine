#pragma once

#include "core/Core.h"
#include "runtime/Entity.h"
#include "systems/SubSystem.h"

struct WaterBodyComponent;

class EngineAPI WaterSystem : public SubSystem
{
public:
	WaterSystem();
	static Entity createPool();

	static void drawWaterBody(const WaterBodyComponent& waterBody);

	void prepareWaterBodyForRender(WaterBodyComponent& waterBody);
};