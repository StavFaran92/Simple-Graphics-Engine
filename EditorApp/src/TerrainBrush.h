#pragma once

#include "sge.h"

// Brush math for a terrain that may be moved, rotated or scaled
namespace TerrainBrush
{
	// Raycast against the terrain where it's drawn; the hit is in world space
	RayHit raycast(Entity entity, Terrain& terrain, const Ray& worldRay);

	// A world point in terrain units, from the terrain's corner (0..width, 0..height)
	glm::vec2 worldToTerrain(Entity entity, Terrain& terrain, const glm::vec3& worldPoint);

	// A world point in a texture's pixels (the heightmap or a layer mask)
	glm::vec2 worldToPixel(Entity entity, Terrain& terrain, const glm::vec3& worldPoint, const TextureResourceRef& texture);
}
