#pragma once

#include "Core.h"

#include <glm/glm.hpp>

#include "Entity.h"

class EngineAPI Physics
{
public:
    struct HitResult
    {
        glm::vec3 position;
        glm::vec3 normal;
        float distance = 0;
        Entity e = Entity::EmptyEntity;
    };

    enum LayerMask
    {
        LAYER_0 = 0,
        LAYER_1 = (1 << 0),
        LAYER_2 = (1 << 1),
        LAYER_3 = (1 << 2),
        LAYER_4 = (1 << 3),
        LAYER_5 = (1 << 4),
        LAYER_6 = (1 << 5),
        LAYER_7 = (1 << 6),
        LAYER_8 = (1 << 7),
        LAYER_9 = (1 << 8),
        LAYER_10 = (1 << 9),
        LAYER_11 = (1 << 10),
        LAYER_12 = (1 << 11),
        LAYER_13 = (1 << 12),
        LAYER_14 = (1 << 13),
        LAYER_15 = (1 << 14),
        LAYER_16 = (1 << 15),
        LAYER_17 = (1 << 16),
    };
public:
	static bool raycast(glm::vec3 origin, glm::vec3 dir, float distance, HitResult& hitResult, LayerMask mask = LayerMask::LAYER_0);
};