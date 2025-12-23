#pragma once 

#include <utility>

#include "core/Core.h"
#include "glm/glm.hpp"

struct AABB;

struct Ray
{
	Ray(glm::vec3 origin, glm::vec3 dir)
		: origin(origin), direction(dir)
	{
	}

	glm::vec3 origin;
	glm::vec3 direction;
};

struct RayHit {
	bool hit = false;
	float t = 0.0f;
	glm::vec3 position{};
	glm::vec3 normal{};
};

class EngineAPI Math3D
{
public:
	static std::pair<glm::vec3, glm::vec3 > ScreenPointToRay(
		float mouseX, float mouseY,
		float viewportWidth, float viewportHeight,
		const glm::mat4& view,
		const glm::mat4& projection
	);

	static bool RayIntersectXZBounds(
		const Ray& ray,
		const AABB& boundingBox,
		float& tEnter,
		float& tExit
	);
};