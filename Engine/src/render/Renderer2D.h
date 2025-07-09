#pragma once

#include "RendererIntermediate.h"

class Model;
class ICamera;

class Renderer2D : public RendererIntermediate
{
public:
	Renderer2D() = default;

	// Inherited via IRenderer
	virtual void render() override;

	// Inherited via IRenderer
	void renderScene(Scene*) override;
};

