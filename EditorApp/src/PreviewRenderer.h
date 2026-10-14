#pragma once

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>

#include "render/FrameBufferObject.h"
#include "render/RenderBufferObject.h"
#include "render/Shader.h"
#include "memory/ResourceRef.h"
#include "memory/AssetAliases.h"
#include "texture/Texture.h"
#include "glm/glm.hpp"

struct PreviewRenderDesc
{
    ModelResourceRef model;

    // Material per mesh material index; missing slots use the default material
    std::unordered_map<int, MaterialResourceRef> materials;

    // Final bone matrices; empty means the model is drawn unanimated
    std::vector<glm::mat4> boneMatrices;

    glm::vec4 clearColor = glm::vec4(0.15f, 0.15f, 0.15f, 1.f);

    // Render upside down, so the target is top-down (row 0 = top) and draws with ImGui's default UVs
    bool flipY = false;
};

// Offscreen PBR render of a single model, with the camera framed on the model's AABB.
// The caller owns the color texture, so re-rendering (e.g. on reimport) overwrites it in place.
class PreviewRenderer
{
public:
    PreviewRenderer();

    // Renders into target (must be color-renderable, e.g. RGB8/RGBA8); size and aspect come from target.
    // Returns false if nothing was rendered.
    bool render(const PreviewRenderDesc& desc, const TextureResourceRef& target);

private:
    FrameBufferObject  m_fbo;
    RenderBufferObject m_rbo;
    ShaderResourceRef m_shader;

    glm::mat4 m_view       = glm::mat4(1.f);
    glm::mat4 m_projection = glm::mat4(1.f);

    // Size of the depth buffer; recreated only when the target size changes
    int m_depthWidth  = 0;
    int m_depthHeight = 0;
};
