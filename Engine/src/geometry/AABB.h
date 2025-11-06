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
	
private:
	glm::vec3 min{};
	glm::vec3 max{};
};