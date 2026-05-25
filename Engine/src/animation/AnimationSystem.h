#pragma once

#include "systems/SubSystem.h"

class Scene;

class EngineAPI AnimationSystem : public SubSystem
{
public:
	AnimationSystem();

	void update(Scene* scene, float dt);
};
