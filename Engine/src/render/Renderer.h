#pragma once

#include "core/Core.h"

class Scene;

class EngineAPI Renderer
{
public:
	static void render(Scene* scene, float deltaTime);
};
