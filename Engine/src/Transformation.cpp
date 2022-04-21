#include "Transformation.h"
#include<glm/gtx/quaternion.hpp>
#include "LinearAlgebraUtil.h"

glm::mat4 Transformation::getTransformation() const
{
	return m_transformation;
}

void Transformation::update(float deltaTime)
{
	if (m_change)
	{
		auto pivot = glm::translate(glm::mat4(1.0f), m_pivot);
		auto pivotInv = glm::translate(glm::mat4(1.0f), -m_pivot);

		glm::mat4 identity(1.0f);
		auto translate = glm::translate(glm::mat4(1.0f), m_translation);
		auto scale = glm::scale(glm::mat4(1.0f), m_scale);

		auto finalRotation = glm::mat4_cast(m_orientationLocal) * pivot * glm::mat4_cast(m_orientationWorld) * pivotInv * identity;

		m_transformation = scale * finalRotation * translate * identity;

		m_change = false;
	}
}
void Transformation::SetPosition(glm::vec3 pos)
{
	m_translation = pos;

	m_change = true;
}
void Transformation::SetRotation(float angle, glm::vec3 axis)
{
	m_change = true;
}
void Transformation::SetScale(glm::vec3 scale)
{
	m_scale = scale;

	m_change = true;
}

glm::vec3 Transformation::getPosition() const
{
	return m_translation;
}

glm::quat Transformation::getLocalOrientation() const
{
	return m_orientationLocal;
}

glm::quat Transformation::getWorldOrientation() const
{
	return m_orientationWorld;
}

glm::vec3 Transformation::getScale() const
{
	return m_scale;
}

void Transformation::translate(float x, float y, float z)
{
	m_translation.x += x;
	m_translation.y += y;
	m_translation.z += z;

	m_change = true;
}

void Transformation::rotate(glm::vec3 eulers)
{
	m_orientationLocal = glm::quat(eulers) * m_orientationLocal;

	m_change = true;
}

void Transformation::rotate(glm::vec3 axis, float angle)
{
	m_orientationLocal = glm::angleAxis(degToRad(angle), axis) * m_orientationLocal;

	m_change = true;
}

void Transformation::rotateAround(glm::vec3 pivot, glm::vec3 axis, float angle)
{
	m_orientationWorld = glm::angleAxis(degToRad(angle), axis) * m_orientationWorld;
	m_pivot = pivot;

	m_change = true;
}