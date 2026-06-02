#pragma once

#include <cstdint>
#include <glm/glm.hpp>

struct RenderData
{
    // per object
    struct ObjectData {
        glm::mat4 model;
        glm::mat4 normalMatrix;
        uint32_t materialIndex;
        uint32_t meshIndex;
        glm::vec2 padding;
    };

    // per material
    struct MaterialData {
        glm::vec4 albedo;
        float roughness;
        float metallic;
        uint32_t albedoTexture;
        uint32_t normalTexture;
    };

    // per draw call
    struct DrawCommand {
        uint32_t indexCount;
        uint32_t instanceCount;
        uint32_t firstIndex;
        uint32_t baseVertex;
        uint32_t baseInstance;
    };
};