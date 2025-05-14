#pragma once

#include "sge.h"

#include <cereal/types/polymorphic.hpp>
#include <cereal/cereal.hpp>

class PlayerController : public ScriptableEntity
{
public:
	PlayerController() = default;

	void onCreate() override
	{
		m_camera = Engine::get()->getContext()->getActiveScene()->getEntityByName("Main Camera");
		m_movementSpeed = 3.f;
	}

	void handleGroundCheck()
	{
		// Check if the player is grounded
		auto& transform = entity.getComponent<Transformation>();
		Physics::HitResult hitResult;
		m_isGrounded = Physics::raycast(transform.getWorldPosition() + glm::vec3(0, -1.1, 0), glm::vec3(0, -1, 0), 1.f, hitResult);
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
		glm::vec3 newPosition = m_movementH + m_movementV + glm::vec3(0, m_velocity.y / 1000.f, 0);
		rb.move(newPosition);
	}

	void handleMoveInput(float deltaTime)
	{
		float velocity = m_movementSpeed * deltaTime;

		auto& camComponent = m_camera.getComponent<CameraComponent>();

		if (Engine::get()->getInput()->getKeyboard()->getKeyState(SDL_SCANCODE_W))
		{
			m_movementH = glm::vec3(camComponent.front.x, 0, camComponent.front.z) * velocity;
		}
		else if (Engine::get()->getInput()->getKeyboard()->getKeyState(SDL_SCANCODE_S))
		{
			m_movementH = -glm::vec3(camComponent.front.x, 0, camComponent.front.z) * velocity;
		}
		else
		{
			m_movementH = glm::vec3(0);
		}

		if (Engine::get()->getInput()->getKeyboard()->getKeyState(SDL_SCANCODE_A))
		{
			m_movementV = -glm::vec3(camComponent.right.x, 0, camComponent.right.z) * velocity;
		}
		else if (Engine::get()->getInput()->getKeyboard()->getKeyState(SDL_SCANCODE_D))
		{
			m_movementV = glm::vec3(camComponent.right.x, 0, camComponent.right.z) * velocity;
		}
		else
		{
			m_movementV = glm::vec3(0);
		}


	}

	void shoot()
	{
		auto& camComponent = m_camera.getComponent<CameraComponent>();
		auto& transform = m_camera.getComponent<Transformation>();

		Physics::HitResult hitResult;
		if (Physics::raycast(transform.getWorldPosition(), camComponent.front, 100.f, hitResult, Physics::LayerMask::LAYER_1))
		{
			hitResult.e.getComponent<RigidBodyComponent>().setForce(-hitResult.normal * m_bulletForce);
		}
	}

	void onUpdate(float deltaTime) override
	{
		
		handleMoveInput(deltaTime);
		handleGroundCheck();
		applyGravity(deltaTime);
		applyMovement(deltaTime);

		Engine::get()->getInput()->getMouse()->onMousePressed(Mouse::MouseButton::LeftMousebutton, [&](SDL_Event e) { shoot(); });

		// Handle jumping
		Engine::get()->getInput()->getKeyboard()->onKeyPressed(SDL_SCANCODE_SPACE, [&](SDL_Event e) { if(m_isGrounded)m_velocity.y = m_jumpForce; });
		
	}

private:
	float m_movementSpeed;
	Entity m_camera;

	bool m_isGrounded = false;
	bool m_isJumping = false;
	float m_jumpForce = 200.f;
	float m_bulletForce = 800.f;

	glm::vec3 m_velocity{};
	glm::vec3 m_movementH{};
	glm::vec3 m_movementV{};
};

CEREAL_REGISTER_TYPE(PlayerController);
CEREAL_REGISTER_POLYMORPHIC_RELATION(ScriptableEntity, PlayerController)