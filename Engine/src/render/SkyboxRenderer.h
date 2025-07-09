#pragma once

#include "render/Renderer.h"

class Material;

class SkyboxRenderer : public Renderer
{
public:
	SkyboxRenderer() = default;

	void render();
};

