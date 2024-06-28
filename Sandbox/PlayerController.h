#pragma once

#include "sge.h"

class PlayerController : public ScriptableEntity
{
public:
	PlayerController(Entity camera, float moveSpeed) : m_camera(camera), m_movementSpeed(moveSpeed) {};

	void onCreate() override
	{
	}

	void handleGroundCheck()
	{
		// Check if the player is grounded
		auto& transform = entity.getComponent<Transformation>();
		m_isGrounded = Physics::raycast(transform.getWorldPosition() + glm::vec3(0, -1.1, 0), glm::vec3(0, -1, 0), 1.f);
	}

	void applyGravity(float deltaTime)
	{
		if (m_isGrounded && m_velocity.y < 0)
		{
			m_velocity.y = 0.f; // Reset vertical velocity when grounded
		}
		
		if (!m_isGrounded)
		{
			m_velocity.y +=  -9.81f; // Apply gravity
		}
	}

	void applyMovement(float deltaTime)
	{
		// Apply the computed velocity to move the character
		auto& transform = entity.getComponent<Transformation>();
		auto& rb = entity.getComponent<RigidBodyComponent>();
		glm::vec3 newPosition = movement + glm::vec3(0, m_velocity.y / 1000.f, 0);
		rb.move(newPosition);
	}

	void onUpdate(float deltaTime) override
	{
		float velocity = m_movementSpeed * deltaTime;

		auto& camComponent = m_camera.getComponent<CameraComponent>();

		if (Engine::get()->getInput()->getKeyboard()->getKeyState(SDL_SCANCODE_W))
		{
			movement = glm::vec3(camComponent.front.x, 0, camComponent.front.z) * velocity;
		}

		else if (Engine::get()->getInput()->getKeyboard()->getKeyState(SDL_SCANCODE_S))
		{
			movement = -glm::vec3(camComponent.front.x, 0, camComponent.front.z) * velocity;
		}

		else if (Engine::get()->getInput()->getKeyboard()->getKeyState(SDL_SCANCODE_A))
		{
			movement = -glm::vec3(camComponent.right.x, 0, camComponent.right.z) * velocity;
		}

		else if (Engine::get()->getInput()->getKeyboard()->getKeyState(SDL_SCANCODE_D))
		{
			movement = glm::vec3(camComponent.right.x, 0, camComponent.right.z) * velocity;
		}

		else
		{
			movement = glm::vec3(0);
		}

		handleGroundCheck();
		applyGravity(deltaTime);
		applyMovement(deltaTime);

		// Handle jumping
		Engine::get()->getInput()->getKeyboard()->onKeyPressed(SDL_SCANCODE_SPACE, [&](SDL_Event e) { if(m_isGrounded)m_velocity.y = m_jumpForce; });
		
	}

private:
	float m_movementSpeed;
	Entity m_camera;

	bool m_isGrounded = false;
	bool m_isJumping = false;
	float m_jumpForce = 200.f;

	glm::vec3 m_velocity{};
	glm::vec3 movement{};
};
