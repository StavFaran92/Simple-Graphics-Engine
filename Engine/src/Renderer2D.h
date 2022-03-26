#pragma once

#include "IRenderer.h"

class Renderer2D : public IRenderer
{
public:
	Renderer2D() = default;
	// Inherited via IRenderer
	virtual void Draw(const VertexArrayObject& vao, std::shared_ptr<Shader> shader) const override;
	virtual void Clear() const override;
};
