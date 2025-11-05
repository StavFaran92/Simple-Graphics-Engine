#pragma once

#include "geometry/AABB.h"
#include "component/Transformation.h"



AABB AABB::createFromCenterExtents(glm::vec3 center, glm::vec3 extents)
{
	AABB aabb;
	aabb.center = center;
	aabb.extents = extents;
	return aabb;
};

AABB AABB::createFromMinMax(glm::vec3 min, glm::vec3 max)
{
	AABB aabb;
	glm::vec3 extents = (max - min) / 2.f;
	aabb.center = min + extents;
	aabb.extents = extents;
	return aabb;
};

bool AABB::isOnFrustum(const Frustum& frustum) const
{
	return isForwardOfPlane(frustum.m_znear) &&
		isForwardOfPlane(frustum.m_zfar) &&
		isForwardOfPlane(frustum.m_left) &&
		isForwardOfPlane(frustum.m_right) &&
		isForwardOfPlane(frustum.m_up) &&
		isForwardOfPlane(frustum.m_down);
}

void AABB::transform(const glm::mat4& transform)
{
	glm::vec4 calcCenter = transform * glm::vec4(center, 1.0f);
	glm::vec4 calcExtent = transform * glm::vec4(center + extents, 1.0f);
	calcExtent -= calcCenter;
	center = { calcCenter.x, calcCenter.y, calcCenter.z };
	extents = { calcExtent.x, calcExtent.y, calcExtent.z };
}

bool AABB::isForwardOfPlane(Plane p) const
{
	// Compute the projection interval radius of b onto L(t) = b.c + t * p.n
	const float r = extents.x * std::abs(p.m_normal.x) +
		extents.y * std::abs(p.m_normal.y) + extents.z * std::abs(p.m_normal.z);

	return -r <= p.getSignedDistanceToPlane(center);
}
