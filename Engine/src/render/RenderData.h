#pragma once

#include <cstdint>
#include <glm/glm.hpp>

struct RenderData
{
    // per object
    struct ObjectData {
        glm::mat4 model;
        glm::mat4 normalMatrix;
        uint32_t materialIndex = 0;
        uint32_t meshIndex = 0;
        glm::vec2 padding;
    };

    // per material
    struct MaterialData {
        uint64_t albedoTexture;
        uint64_t normalTexture;
        glm::vec4 baseColor;
    };

    // per draw call
    struct DrawCommand {
        uint32_t vertexCount;
        uint32_t instanceCount;
        uint32_t firstIndex;
        uint32_t baseVertex;
        uint32_t baseInstance;
    };
};