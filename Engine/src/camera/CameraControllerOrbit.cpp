#include "camera/CameraControllerOrbit.h"

#include "core/Engine.h"
#include "core/EventSystem.h"
#include "core/ApplicationConstants.h"
#include "component/Transformation.h"
#include <algorithm>
#include "glm/glm.hpp"
#include "core/MouseEvents.h"

//void CameraControllerOrbit::calculateOrientation()
//{
//	float t = m_distance * cos(m_angleY * Constants::toRadians);
//	float y = m_distance * sin(m_angleY * Constants::toRadians);
//	float x = t * cos(m_angleX * Constants::toRadians);
//	float z = t * sin(m_angleX * Constants::toRadians);
//
//	m_cameraTransform->setLocalPosition(glm::vec3(x, y, z) + m_cameraComponent->center);
//
//	auto front = glm::normalize(-m_cameraTransform->getLocalPosition() + m_cameraComponent->center);
//	m_right = glm::normalize(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f)));
//	m_cameraComponent->up = glm::normalize(glm::cross(m_right, front));
//}

void CameraControllerOrbit::onCreate(Entity& e)
{
	m_cameraComponent = &e.getComponent<CameraComponent>();
	m_cameraTransform = &e.getComponent<Transformation>();
}

bool CameraControllerOrbit::onEvent(const Event& e)
{
	if (e.type() == EventType::MouseMoved)
	{
		const auto& me = static_cast<const MouseMovedEvent&>(e);
		if (m_state == ControllerState::ROTATE)
		{
			m_angleX += me.xrel * m_turnSpeed;
			m_angleY += me.yrel * m_turnSpeed;

			m_angleY = std::clamp(m_angleY, -89.f, 89.f);
		}
		else if (m_state == ControllerState::TRANSFORM)
		{
			float xVelocity = .1f * (me.xrel * m_movementSpeed);
			float yVelocity = .1f * (me.yrel * m_movementSpeed);

			m_cameraComponent->center += m_right * xVelocity;
			m_cameraComponent->center -= m_cameraComponent->up * yVelocity;
		}
	}
	else if (e.type() == EventType::MouseButtonPressed)
	{
		const auto& me = static_cast<const MouseButtonPressedEvent&>(e);

		if (me.button == MOUSE_BUTTON_RIGHT)
		{
			if (m_state == ControllerState::IDLE)
				m_state = ControllerState::ROTATE;
		}
		else if (me.button == MOUSE_BUTTON_MIDDLE)
		{
			if (m_state == ControllerState::IDLE)
				m_state = ControllerState::TRANSFORM;
		}
	}
	else if (e.type() == EventType::MouseButtonReleased)
	{
		const auto& me = static_cast<const MouseButtonReleasedEvent&>(e);

		if (me.button == MOUSE_BUTTON_RIGHT || me.button == MOUSE_BUTTON_MIDDLE)
			m_state = ControllerState::IDLE;
	}
	else if (e.type() == EventType::MouseWheel)
	{
		const auto& me = static_cast<const MouseWheelEvent&>(e);
		m_distance = std::clamp(m_distance - static_cast<float>(me.y), 1.f, 50.f);
	}

	return false;
}
