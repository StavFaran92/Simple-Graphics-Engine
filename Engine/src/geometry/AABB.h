#pragma once

#include "Volume.h"

class Transformation;

struct AABB : public Volume
{
	AABB() = default;

	static AABB createFromCenterExtents(glm::vec3 center, glm::vec3 extents);

	static AABB createFromMinMax(glm::vec3 min, glm::vec3 max);

	virtual bool isOnFrustum(const Frustum& frustum) const;

	void adjustToTransform(const Transformation& transform);

	bool isForwardOfPlane(Plane p) const;
	

	glm::vec3 center{};
	glm::vec3 extents{};
};