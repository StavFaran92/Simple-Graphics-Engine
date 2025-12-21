#include "camera/CameraControllerFreeLook.h"

#include "core/Engine.h"
#include "core/EventSystem.h"
#include "core/ApplicationConstants.h"
#include "component/Transformation.h"
#include "core/System.h"
#include "ui/Input.h"
#include <algorithm>
#include "glm/glm.hpp"
#include "core/Window.h"

void CameraControllerFreeLook::onCreate(Entity& e)
{
	m_cameraComponent = &e.getComponent<CameraComponent>();
	m_cameraTransform = &e.getComponent<Transformation>();

	
}

void CameraControllerFreeLook::onUpdate(float deltaTime)
{
	glm::vec3 movement(0.0f);

	if (keyState[KeyCode::SCANCODE_W])
		movement += m_cameraComponent->front;
	if (keyState[KeyCode::SCANCODE_S])
		movement -= m_cameraComponent->front;

	if (keyState[KeyCode::SCANCODE_D])
		movement += m_cameraComponent->right;
	if (keyState[KeyCode::SCANCODE_A])
		movement -= m_cameraComponent->right;

	if (keyState[KeyCode::SCANCODE_E])
		movement.y += 1.0f;
	if (keyState[KeyCode::SCANCODE_Q])
		movement.y -= 1.0f;

	if (glm::length(movement) > 0.0001f)
		movement = glm::normalize(movement) * m_movementSpeed * deltaTime;

	m_cameraTransform->translate(movement);
}

bool CameraControllerFreeLook::onEvent(SDL_Event e)
{
	if (e.type == SDL_MOUSEMOTION)
	{
		auto system = Engine::get()->getSubSystem<System>();

		if (m_state == ControllerState::ROTATE)
		{
			float xChange = e.motion.xrel;
			float yChange = e.motion.yrel;

			xChange *= m_turnSpeed * system->getDeltaTime();
			yChange *= m_turnSpeed * system->getDeltaTime();

			m_yaw -= xChange;
			m_pitch -= yChange;

			if (m_pitch > 89.0f)
			{
				m_pitch = 89.0f;
			}

			if (m_pitch < -89.0f)
			{
				m_pitch = -89.0f;
			}

			glm::quat pitchQuat = glm::angleAxis(glm::radians(m_pitch), glm::vec3(1, 0, 0));
			glm::quat yawQuat = glm::angleAxis(glm::radians(m_yaw), glm::vec3(0, 1, 0));

			// Combine the quaternions
			glm::quat combinedQuat = yawQuat * pitchQuat;

			m_cameraTransform->setWorldRotation(combinedQuat);
		}

		if (m_state == ControllerState::TRANSFORM)
		{
			auto system = Engine::get()->getSubSystem<System>();

			float xChange = e.motion.xrel;
			float yChange = e.motion.yrel;

			xChange *= m_movementSpeed * system->getDeltaTime();
			yChange *= m_movementSpeed * system->getDeltaTime();

			float xVelocity = .1f * xChange;
			float yVelocity = .1f * yChange;


			m_cameraTransform->translate(m_cameraComponent->right * xVelocity);
			m_cameraTransform->translate(-m_cameraComponent->up * yVelocity);
		}
	}
	else if (e.type == SDL_MOUSEBUTTONDOWN)
	{
		if (e.button.button == SDL_BUTTON_RIGHT)
		{
			if (m_state == ControllerState::IDLE)
			{
				m_state = ControllerState::ROTATE;
				Engine::get()->getWindow()->lockMouse();
			}
		}

		else if (e.button.button == SDL_BUTTON_MIDDLE)
		{
			if (m_state == ControllerState::IDLE)
			{
				m_state = ControllerState::TRANSFORM;
				Engine::get()->getWindow()->lockMouse();
			}
		}
	}
	else if (e.type == SDL_MOUSEBUTTONUP)
	{
		if (e.button.button == SDL_BUTTON_RIGHT || e.button.button == SDL_BUTTON_MIDDLE)
		{
			m_state = ControllerState::IDLE;
			Engine::get()->getWindow()->unlockMouse();
		}
	}
	else if (e.type == SDL_MOUSEWHEEL)
	{
		m_cameraTransform->translate(m_cameraComponent->front * (float)e.wheel.y);
	}
	else if (e.type == SDL_KEYDOWN && !e.key.repeat)
	{
		auto it = keyState.find(static_cast<KeyCode>(e.key.keysym.scancode));
		if (it != keyState.end())
			it->second = true;

		return false;
	}

	else if (e.type == SDL_KEYUP)
	{
		auto it = keyState.find(static_cast<KeyCode>(e.key.keysym.scancode));
		if (it != keyState.end())
			it->second = false;

		return false;
	}

	return false;
}
