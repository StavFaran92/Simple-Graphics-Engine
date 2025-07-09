#pragma once

#include "Plane.h"

struct Frustum
{
	Frustum(glm::vec3 pos, glm::vec3 front, glm::vec3 up, glm::vec3 right,
		float aspect, float fovy, float znear, float zfar)
	{
		// calculate HalfVSide and HalfHSide
		float halfVSide = zfar * tanf(fovy * .5f);
		float halfHSide = halfVSide * aspect;
		glm::vec3 frontMultFar = zfar * front;

		// calculate planes
		m_znear = { pos + znear * front, front };
		m_zfar = { pos + frontMultFar,  -front };
		m_right = { pos, glm::cross(up, frontMultFar + halfHSide * right) };
		m_left = { pos, -glm::cross(up, frontMultFar - halfHSide * right) };
		m_up = { pos, -glm::cross(right, frontMultFar + halfVSide * up) };
		m_down = { pos, glm::cross(right, frontMultFar - halfVSide * up) };
	}

	Plane m_znear;
	Plane m_zfar;
	Plane m_right;
	Plane m_left;
	Plane m_up;
	Plane m_down;
};