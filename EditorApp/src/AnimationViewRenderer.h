#pragma once

#include <cstdint>
#include <memory>

#include "render/FrameBufferObject.h"
#include "render/RenderBufferObject.h"
#include "render/Shader.h"
#include "memory/ResourceRef.h"
#include "memory/AssetAliases.h"
#include "texture/Texture.h"
#include "glm/glm.hpp"

class Animator;
class Animation;
class MeshRendererComponent;

class AnimationViewRenderer
{
public:
    AnimationViewRenderer();

    void resize(uint32_t width, uint32_t height);
    void render(AnimationAssetRef animation, float currentTime, MeshRendererComponent* meshRendererComponent);

    uint32_t getColorTextureID() const;
    uint32_t getWidth()  const { return m_width; }
    uint32_t getHeight() const { return m_height; }

private:
    FrameBufferObject  m_fbo;
    RenderBufferObject m_rbo;
    TextureResourceRef m_colorTexture;
    ShaderResourceRef m_shader;

    glm::mat4 m_view       = glm::mat4(1.f);
    glm::mat4 m_projection = glm::mat4(1.f);

    uint32_t m_width  = 0;
    uint32_t m_height = 0;
};
