#pragma once
#include "render/Shader.h"
#include "memory/ResourceRef.h"
#include "texture/Texture.h"
#include "core/Core.h"

class EngineAPI TexturePainter
{
public:
    enum class DrawMode
    {
        Add,
        Subtract
    };

    void setTexture(const TextureResourceRef& tex);

    void setBrushRadius(float r);
    float getBrushRadius() const;

    void setBrushStrength(float s);
    float getBrushStrength()const;

    void setDrawMode(DrawMode mode);
    DrawMode getDrawMode() const;

    // Values the brush clamps to; 0..1 by default, for masks
    void setValueRange(float min, float max);

    // brushCenter is in pixel coordinates
    void applyBrush(int pixelX, int pixelY);



    // Read back the entire texture into CPU memory (for serialization)
    std::vector<float> readTextureData();

    TextureResourceRef getBoundTexture();

private:
    TextureResourceRef m_texture;
    float m_brushRadius = 10.0f;
    float m_brushStrength = .1f;
    float m_valueMin = 0.f;
    float m_valueMax = 1.f;

    DrawMode m_drawMode = DrawMode::Add;

};