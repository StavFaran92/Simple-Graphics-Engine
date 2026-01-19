#pragma once

#include "core/Core.h"
#include "component/Volume.h"

class Transformation;

struct EngineAPI AABB : public Volume
{
	AABB() = default;

	static AABB createFromCenterExtents(glm::vec3 center, glm::vec3 extents);

	static AABB createFromMinMax(glm::vec3 min, glm::vec3 max);

	virtual bool isOnFrustum(const Frustum& frustum) const;

	void transform(const glm::mat4& transform);

	bool isForwardOfPlane(Plane p) const;

	glm::vec3 center() const;
	glm::vec3 extents() const;

	float minX() const { return min.x; }
	float minY() const { return min.y; }
	float minZ() const { return min.z; }

	float maxX() const { return max.x; }
	float maxY() const { return max.y; }
	float maxZ() const { return max.z; }

	glm::vec3 getMin() const { return min; }
	glm::vec3 getMax() const { return max; }
	
private:
	glm::vec3 min{};
	glm::vec3 max{};
};