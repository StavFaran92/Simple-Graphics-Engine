#pragma once

#include "geometry/AABB.h"
#include "component/Transformation.h"



AABB AABB::createFromCenterExtents(glm::vec3 center, glm::vec3 extents)
{
	AABB aabb;
	aabb.min = center - extents / 2.f;
	aabb.max = center + extents / 2.f;
	return aabb;
};

AABB AABB::createFromMinMax(glm::vec3 min, glm::vec3 max)
{
	AABB aabb;
	aabb.min = min;
	aabb.max = max;
	return aabb;
};

bool AABB::isOnFrustum(const Frustum& frustum) const
{
	bool isForwardOfNear = isForwardOfPlane(frustum.m_znear);
	bool isForwardOfFar = isForwardOfPlane(frustum.m_zfar);
	bool isForwardOfLeft = isForwardOfPlane(frustum.m_left);
	bool isForwardOfRight = isForwardOfPlane(frustum.m_right);
	bool isForwardOfUp = isForwardOfPlane(frustum.m_up);
	bool isForwardOfDown = isForwardOfPlane(frustum.m_down);

	bool isOnFrustum = isForwardOfNear && isForwardOfFar && isForwardOfLeft && isForwardOfRight && isForwardOfUp && isForwardOfDown;

	return isOnFrustum;
}

void AABB::transform(const glm::mat4& transform)
{
	// This is incorrect, but is fine for now...

	glm::vec3 c = center();
	glm::vec3 e = extents();

	glm::vec3 corners[8] = {
		c + glm::vec3(-e.x, -e.y, -e.z),
		c + glm::vec3(e.x, -e.y, -e.z),
		c + glm::vec3(-e.x,  e.y, -e.z),
		c + glm::vec3(e.x,  e.y, -e.z),
		c + glm::vec3(-e.x, -e.y,  e.z),
		c + glm::vec3(e.x, -e.y,  e.z),
		c + glm::vec3(-e.x,  e.y,  e.z),
		c + glm::vec3(e.x,  e.y,  e.z)
	};

	glm::vec3 newMin(std::numeric_limits<float>::max());
	glm::vec3 newMax(-std::numeric_limits<float>::max());

	for (int i = 0; i < 8; ++i)
	{
		glm::vec3 transformed = glm::vec3(transform * glm::vec4(corners[i], 1.0f));
		newMin = glm::min(newMin, transformed);
		newMax = glm::max(newMax, transformed);
	}

	min = newMin;
	max = newMax;
}

bool AABB::isForwardOfPlane(Plane p) const
{
	glm::vec3 center = (max + min) / 2.f;
	glm::vec3 extents = max - center;

	// Compute the projection interval radius of b onto L(t) = b.c + t * p.n
	const float r = extents.x * std::abs(p.m_normal.x) +
		extents.y * std::abs(p.m_normal.y) + extents.z * std::abs(p.m_normal.z);

	return -r <= p.getSignedDistanceToPlane(center);
}

glm::vec3 AABB::center() const
{
	return (min + max) * 0.5f;
}

glm::vec3 AABB::extents() const
{
	return (max - min) * 0.5f;
}
