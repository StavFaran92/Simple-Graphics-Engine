#include "CameraControllerFreeLook.h"

#include "Engine.h"
#include "EventSystem.h"
#include "ApplicationConstants.h"
#include "Transformation.h"
#include "System.h"
#include "Input.h"
#include <algorithm>
#include "glm/glm.hpp"

void CameraControllerFreeLook::onCreate(Entity& e)
{
	m_cameraComponent = &e.getComponent<CameraComponent>();
	m_cameraTransform = &e.getComponent<Transformation>();

	auto eventSystem = Engine::get()->getEventSystem();

	m_handlerID = eventSystem->bindToLayer("GameLayer"); // TODO fix

	eventSystem->subscribe(m_handlerID, SDL_MOUSEMOTION, [this](SDL_Event e)
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
		});
	eventSystem->subscribe(m_handlerID, SDL_MOUSEBUTTONDOWN, [this](SDL_Event e){
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
	});
	eventSystem->subscribe(m_handlerID, SDL_MOUSEBUTTONUP, [this](SDL_Event e){
		if (e.button.button == SDL_BUTTON_RIGHT || e.button.button == SDL_BUTTON_MIDDLE)
		{
			m_state = ControllerState::IDLE;
		}
	});
	eventSystem->subscribe(m_handlerID, SDL_MOUSEWHEEL, [this](SDL_Event e){
		m_cameraTransform->translate(m_cameraComponent->front * (float)e.wheel.y);
	});

	
}

void CameraControllerFreeLook::onUpdate(float deltaTime)
{
	if(Engine::get()->getInput()->getKeyboard()->getKeyState(SDL_SCANCODE_W))
	{
		m_velocityF = m_cameraComponent->front * velocity * deltaTime;
	}
	else if (Engine::get()->getInput()->getKeyboard()->getKeyState(SDL_SCANCODE_S))
	{
		m_velocityF = -m_cameraComponent->front * velocity * deltaTime;
	}
	else
	{
		m_velocityF = glm::vec3{ 0 };
	}

	if (Engine::get()->getInput()->getKeyboard()->getKeyState(SDL_SCANCODE_D))
	{
		m_velocityR = m_cameraComponent->right * velocity * deltaTime;
	}
	else if (Engine::get()->getInput()->getKeyboard()->getKeyState(SDL_SCANCODE_A))
	{
		m_velocityR = -m_cameraComponent->right * velocity * deltaTime;
	}
	else
	{
		m_velocityR = glm::vec3{ 0 };
	}

	if (Engine::get()->getInput()->getKeyboard()->getKeyState(SDL_SCANCODE_E))
	{
		m_velocityU = glm::vec3{0,1,0} *velocity * deltaTime;
	}
	else if (Engine::get()->getInput()->getKeyboard()->getKeyState(SDL_SCANCODE_Q))
	{
		m_velocityU = glm::vec3{ 0,-1,0 }  * velocity * deltaTime;
	}
	else
	{
		m_velocityU = glm::vec3{ 0 };
	}

	auto finalMovement = m_velocityF + m_velocityR + m_velocityU;

	m_cameraTransform->translate(finalMovement);
}
