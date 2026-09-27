#pragma once

#include "systems/SubSystem.h"

class Scene;

class SpawnedTagCleanupSystem : public SubSystem
{
public:
	SpawnedTagCleanupSystem();

	void update(Scene* scene);
};
