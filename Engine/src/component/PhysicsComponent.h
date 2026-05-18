#pragma once

#include "component/Component.h"
#include "component/ComponentSerializer.h"
#include "physics/Physics.h"

struct EngineAPI PhysicsComponent : public Component
{

	PhysicsComponent() = default;

	std::string getName() override { return "PhysicsComponent"; }

	void addForce(glm::vec3 force);
	void setForce(glm::vec3 force);

	void move(glm::vec3 position);

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(type);
		SERIALIZED_MEMBER(mass);
		SERIALIZED_MEMBER(collider);
		SERIALIZED_MEMBER(colliderType);
		SERIALIZED_MEMBER_OPTIONAL(isLockedLinearX);
		SERIALIZED_MEMBER_OPTIONAL(isLockedLinearY);
		SERIALIZED_MEMBER_OPTIONAL(isLockedLinearZ);
		SERIALIZED_MEMBER_OPTIONAL(isLockedAngularX);
		SERIALIZED_MEMBER_OPTIONAL(isLockedAngularY);
		SERIALIZED_MEMBER_OPTIONAL(isLockedAngularZ);
		SERIALIZED_MEMBER_OPTIONAL(offset);
	}

	bool isLockedLinearX = false;
	bool isLockedLinearY = false;
	bool isLockedLinearZ = false;
	bool isLockedAngularX = false;
	bool isLockedAngularY = false;
	bool isLockedAngularZ = false;
	RigidbodyType type = RigidbodyType::Static;
	float mass = 0;
	bool isChanged = false;
	glm::vec3 m_targetPisition{ 0 };
	glm::vec3 m_force{ 0 };
	void* simulatedBody = nullptr;

	std::shared_ptr<Collider> collider;
	ColliderType colliderType = ColliderType::NONE;
	glm::vec3 offset{};
};

REGISTER_COMPONENT(PhysicsComponent)