#pragma once

#include "AABB.h"
#include "Transformation.h"



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

void AABB::adjustToTransform(const Transformation& transform)
{
	auto& scale = transform.getWorldScale();

	auto& right = transform.getRight() * extents.x * scale.x;
	auto& up = transform.getUp() * extents.y * scale.y;
	auto& front = transform.getForward() * extents.z * scale.z;

	float extentX = std::abs(right[0]) + std::abs(up[0]) + std::abs(front[0]);
	float extentY = std::abs(right[1]) + std::abs(up[1]) + std::abs(front[1]);
	float extentZ = std::abs(right[2]) + std::abs(up[2]) + std::abs(front[2]);

	glm::vec4 calcCenter = transform.getWorldTransformation() * glm::vec4(center, 1.0f);
	center = { calcCenter.x, calcCenter.y, calcCenter.z };
	extents = { extentX, extentY, extentZ };
}

bool AABB::isForwardOfPlane(Plane p) const
{
	// Compute the projection interval radius of b onto L(t) = b.c + t * p.n
	const float r = extents.x * std::abs(p.m_normal.x) +
		extents.y * std::abs(p.m_normal.y) + extents.z * std::abs(p.m_normal.z);

	return -r <= p.getSignedDistanceToPlane(center);
}
