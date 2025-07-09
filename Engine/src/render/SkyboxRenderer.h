#pragma once

#include "Renderer.h"

class Material;

class SkyboxRenderer : public Renderer
{
public:
	SkyboxRenderer() = default;

	void render();
};

