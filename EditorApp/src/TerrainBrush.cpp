#include "TerrainBrush.h"

namespace
{
	// local -> world, as the renderer draws the terrain
	glm::mat4 modelOf(Entity entity)
	{
		return entity.getComponent<Transformation>().getWorldTransformation();
	}
}

RayHit TerrainBrush::raycast(Entity entity, Terrain& terrain, const Ray& worldRay)
{
	const glm::mat4 model = modelOf(entity);
	const glm::mat4 toLocal = glm::inverse(model);

	// Origin is a point (moved), direction a vector (only rotated and scaled)
	const glm::vec3 localOrigin = glm::vec3(toLocal * glm::vec4(worldRay.origin, 1.f));
	const glm::vec3 localDir = glm::vec3(toLocal * glm::vec4(worldRay.direction, 0.f));

	RayHit hit = terrain.raycast(Ray(localOrigin, localDir));
	if (hit.hit)
		hit.position = glm::vec3(model * glm::vec4(hit.position, 1.f));

	return hit;
}

glm::vec2 TerrainBrush::worldToTerrain(Entity entity, Terrain& terrain, const glm::vec3& worldPoint)
{
	const glm::vec3 local = glm::vec3(glm::inverse(modelOf(entity)) * glm::vec4(worldPoint, 1.f));

	// The terrain is centred on its origin
	return { local.x + terrain.getWidth() * .5f, local.z + terrain.getHeight() * .5f };
}

glm::vec2 TerrainBrush::worldToPixel(Entity entity, Terrain& terrain, const glm::vec3& worldPoint, const TextureResourceRef& texture)
{
	const glm::vec2 terrainPos = worldToTerrain(entity, terrain, worldPoint);

	// The texture's resolution may differ from the terrain's size
	return {
		terrainPos.x * texture->getWidth() / terrain.getWidth(),
		terrainPos.y * texture->getHeight() / terrain.getHeight()
	};
}
