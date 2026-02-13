#include "Math3D.h"

#include "geometry/AABB.h"

std::pair<glm::vec3, glm::vec3> Math3D::ScreenPointToRay(float mouseX, float mouseY, float viewportWidth, float viewportHeight, const glm::mat4& view, const glm::mat4& projection)
{
	// 1. NDC
	glm::vec2 ndc;
	ndc.x = (mouseX * 2.0) / viewportWidth - 1.0f;
	ndc.y = -((mouseY * 2.0) / viewportHeight - 1.0f);

	// 2. Clip space
	glm::vec4 rayClip(ndc, -1.0f, 1.0f);

	// 3. View space
	glm::vec4 rayView = glm::inverse(projection) * rayClip;
	rayView = glm::vec4(rayView.x, rayView.y, -1.0f, 0.0f);

	// 4. World space
	glm::vec3 rayDir = glm::normalize(glm::vec3(glm::inverse(view) * rayView));

	// 5. Origin
	glm::vec3 rayOrigin = glm::vec3(glm::inverse(view)[3]); // todo use camera pos here

	return { rayOrigin, rayDir };
}

bool Math3D::RayIntersectXZBounds(
	const Ray& ray,
	const AABB& boundingBox,
	float& tEnter,
	float& tExit
) {
	tEnter = 0.0f;
	tExit = std::numeric_limits<float>::infinity();

	auto slab = [&](float origin, float dir, float minB, float maxB) {
		if (std::abs(dir) < 1e-6f) {
			if (origin < minB || origin > maxB)
				return false;
			return true;
		}

		float invD = 1.0f / dir;
		float t0 = (minB - origin) * invD;
		float t1 = (maxB - origin) * invD;
		if (t0 > t1) std::swap(t0, t1);

		tEnter = std::max(tEnter, t0);
		tExit = std::min(tExit, t1);
		return tEnter <= tExit;
		};

	if (!slab(ray.origin.x, ray.direction.x, boundingBox.minX(), boundingBox.maxX())) return false;
	if (!slab(ray.origin.z, ray.direction.z, boundingBox.minZ(), boundingBox.maxZ())) return false;

	return true;
}