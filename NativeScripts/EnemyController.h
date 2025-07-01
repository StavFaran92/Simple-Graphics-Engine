#pragma once

#include "sge.h"

#include <cereal/types/polymorphic.hpp>
#include <cereal/cereal.hpp>

class EnemyController : public ScriptableEntity
{
public:
	EnemyController() = default;

	void onCreate() override
	{
		m_player = Engine::get()->getContext()->getActiveScene()->getEntityByName("Player");
	}

	void onUpdate(float deltaTime) override
	{
		float velocity = m_movementSpeed * deltaTime;

		// follow the player
		auto& transform = entity.getComponent<Transformation>();
		auto& playerTransform = m_player.getComponent<Transformation>();
		auto& dir = glm::normalize(playerTransform.getWorldPosition() - transform.getWorldPosition());

		m_movement = glm::vec3(dir.x, 0.f, dir.z) * m_speed;

		auto& rb = entity.getComponent<PhysicsComponent>();
		rb.addForce(m_movement);
		
	}

private:
	float m_movementSpeed = 5.0f;

	Entity m_player;

	float m_speed = 5.f;

	bool m_isGrounded = false;
	bool m_isJumping = false;
	float m_jumpForce = 200.f;

	glm::vec3 m_velocity{};
	glm::vec3 m_movement{};
};

CEREAL_REGISTER_TYPE(EnemyController);
CEREAL_REGISTER_POLYMORPHIC_RELATION(ScriptableEntity, EnemyController)