#include "camera/CameraControllerOrbit.h"

#include "core/Engine.h"
#include "core/EventSystem.h"
#include "core/ApplicationConstants.h"
#include "component/Transformation.h"
#include <algorithm>
#include "glm/glm.hpp"

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

void CameraControllerOrbit::onEvent(SDL_Event e)
{
	if (e.type == SDL_MOUSEMOTION)
	{
		int xChange = e.motion.xrel;
		int yChange = e.motion.yrel;
		if (m_state == ControllerState::ROTATE)
		{
			m_angleX += xChange * m_turnSpeed;
			m_angleY += yChange * m_turnSpeed;

			m_angleY = std::clamp(m_angleY, -89.f, 89.f);
		}
		else if (m_state == ControllerState::TRANSFORM)
		{
			int xChange = e.motion.xrel;
			int yChange = e.motion.yrel;

			xChange *= m_movementSpeed;
			yChange *= m_movementSpeed;

			float xVelocity = .1f * xChange;// *deltaTime // todo fix
			float yVelocity = .1f * yChange;// *deltaTime


			m_cameraComponent->center += m_right * xVelocity;
			m_cameraComponent->center -= m_cameraComponent->up * yVelocity;
		}
	}
	else if (e.type == SDL_MOUSEBUTTONDOWN)
	{

		if (e.button.button == SDL_BUTTON_RIGHT)
		{
			if (m_state == ControllerState::IDLE)
			{
				m_state = ControllerState::ROTATE;
			}
		}

		else if (e.button.button == SDL_BUTTON_MIDDLE)
		{
			if (m_state == ControllerState::IDLE)
			{
				m_state = ControllerState::TRANSFORM;
			}
		}
	}
	else if (e.type == SDL_MOUSEBUTTONUP)
	{
		if (e.button.button == SDL_BUTTON_RIGHT || e.button.button == SDL_BUTTON_MIDDLE)
		{
			m_state = ControllerState::IDLE;
		}
	}
	else if (e.type == SDL_MOUSEWHEEL)
	{
		m_distance = std::clamp(m_distance - e.wheel.y, 1.f, 50.f);
	}

}
