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
#include "core/MouseEvents.h"

void CameraControllerFreeLook::onCreate(Entity& e)
{
	m_cameraComponent = &e.getComponent<CameraComponent>();
	m_cameraTransform = &e.getComponent<Transformation>();

	
}

void CameraControllerFreeLook::onUpdate(float deltaTime)
{
	auto* mouse = Engine::get()->getInput()->getMouse();
	bool rmb = mouse->getMouseState().rmb;

	if (!rmb)
	{
		if (m_state != ControllerState::IDLE)
		{
			m_state = ControllerState::IDLE;
			Engine::get()->getWindow()->unlockMouse();
		}
	}

	auto* kb = Engine::get()->getInput()->getKeyboard();
	glm::vec3 dir(0.0f);

	if (kb->getKeyState(KeyCode::SCANCODE_W))
		dir += m_cameraComponent->front;
	if (kb->getKeyState(KeyCode::SCANCODE_S))
		dir -= m_cameraComponent->front;

	if (kb->getKeyState(KeyCode::SCANCODE_D))
		dir += m_cameraComponent->right;
	if (kb->getKeyState(KeyCode::SCANCODE_A))
		dir -= m_cameraComponent->right;

	if (kb->getKeyState(KeyCode::SCANCODE_E))
		dir.y += 1.0f;
	if (kb->getKeyState(KeyCode::SCANCODE_Q))
		dir.y -= 1.0f;

	if (glm::length(dir) > 0.001f)
		dir = glm::normalize(dir) * m_movementSpeed * deltaTime;

	m_cameraTransform->translate(dir);
}

bool CameraControllerFreeLook::onEvent(const Event& e)
{
	// We only allow for state enter and TRS modify to be event based,
	// state leave should ALWAYS be handled in onUpdate.
	if (e.type() == EventType::MouseMoved)
	{
		auto system = Engine::get()->getSubSystem<System>();
		const auto& me = static_cast<const MouseMovedEvent&>(e);

		if (m_state == ControllerState::ROTATE)
		{
			float xChange = static_cast<float>(me.xrel);
			float yChange = static_cast<float>(me.yrel);

			xChange *= m_turnSpeed * system->getDeltaTime();
			yChange *= m_turnSpeed * system->getDeltaTime();

			m_yaw -= xChange;
			m_pitch -= yChange;

			if (m_pitch > 89.0f)
				m_pitch = 89.0f;

			if (m_pitch < -89.0f)
				m_pitch = -89.0f;

			glm::quat pitchQuat = glm::angleAxis(glm::radians(m_pitch), glm::vec3(1, 0, 0));
			glm::quat yawQuat = glm::angleAxis(glm::radians(m_yaw), glm::vec3(0, 1, 0));

			glm::quat combinedQuat = yawQuat * pitchQuat;

			m_cameraTransform->setWorldRotation(combinedQuat);
		}

		if (m_state == ControllerState::TRANSFORM)
		{
			float xChange = static_cast<float>(me.xrel);
			float yChange = static_cast<float>(me.yrel);

			xChange *= m_movementSpeed * system->getDeltaTime();
			yChange *= m_movementSpeed * system->getDeltaTime();

			float xVelocity = .1f * xChange;
			float yVelocity = .1f * yChange;

			m_cameraTransform->translate(m_cameraComponent->right * xVelocity);
			m_cameraTransform->translate(-m_cameraComponent->up * yVelocity);
		}
	}
	else if (e.type() == EventType::MouseButtonPressed)
	{
		const auto& me = static_cast<const MouseButtonPressedEvent&>(e);

		if (me.button == MOUSE_BUTTON_RIGHT)
		{
			if (m_state == ControllerState::IDLE)
			{
				m_state = ControllerState::ROTATE;
				Engine::get()->getWindow()->lockMouse();
			}
		}
		else if (me.button == MOUSE_BUTTON_MIDDLE)
		{
			if (m_state == ControllerState::IDLE)
			{
				m_state = ControllerState::TRANSFORM;
				Engine::get()->getWindow()->lockMouse();
			}
		}
	}
	else if (e.type() == EventType::MouseWheel)
	{
		const auto& me = static_cast<const MouseWheelEvent&>(e);
		m_cameraTransform->translate(m_cameraComponent->front * static_cast<float>(me.y));
	}

	return false;
}
