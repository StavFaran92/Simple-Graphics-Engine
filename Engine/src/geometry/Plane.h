#pragma once

#include <glm/glm.hpp>

struct Plane
{
public:
	Plane() = default;

	Plane(const glm::vec3& pos, const glm::vec3& normal)
		: m_normal(normalize(normal)), 
		m_distance(glm::dot(m_normal, pos))
	{

	}

	float getSignedDistanceToPlane(glm::vec3 p)
	{
		return glm::dot(m_normal, p) - m_distance;
	}

	glm::vec3 m_normal;

	float m_distance;
};