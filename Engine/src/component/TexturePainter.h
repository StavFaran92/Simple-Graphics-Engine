#pragma once
#include "render/Shader.h"
#include "memory/ResourceWrapper.h"
#include "texture/Texture.h"
#include "core/Core.h"

class EngineAPI TexturePainter
{
public:
    void init();
    void setTexture(const ResourceWrapper<Texture>& tex);
    void setBrushRadius(float r);
    void setBrushStrength(float s);

    // brushCenter is in pixel coordinates
    void applyBrush(int pixelX, int pixelY);

    // Read back the entire texture into CPU memory (for serialization)
    std::vector<float> readTextureData();

private:
    ResourceWrapper<Shader> m_texturePaintShader;

    ResourceWrapper<Texture> m_texture;
    float m_brushRadius = 20.0f;
    float m_brushStrength = 1.0f;

};