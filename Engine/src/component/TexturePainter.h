#pragma once
#include "render/Shader.h"
#include "memory/ResourceRef.h"
#include "texture/Texture.h"
#include "core/Core.h"

class EngineAPI TexturePainter
{
public:
    void setTexture(const TextureResourceRef& tex);

    void setBrushRadius(float r);
    float getBrushRadius() const;

    void setBrushStrength(float s);
    float getBrushStrength()const;

    // brushCenter is in pixel coordinates
    void applyBrush(int pixelX, int pixelY);

    // Read back the entire texture into CPU memory (for serialization)
    std::vector<float> readTextureData();

    TextureResourceRef getBoundTexture();

private:
    TextureResourceRef m_texture;
    float m_brushRadius = 10.0f;
    float m_brushStrength = .1f;

};