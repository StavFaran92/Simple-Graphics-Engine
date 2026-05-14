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
		SERIALIZED_MEMBER_OPTIONAL(isLockedLinearX, false);
		SERIALIZED_MEMBER_OPTIONAL(isLockedLinearY, false);
		SERIALIZED_MEMBER_OPTIONAL(isLockedLinearZ, false);
		SERIALIZED_MEMBER_OPTIONAL(isLockedAngularX, false);
		SERIALIZED_MEMBER_OPTIONAL(isLockedAngularY, false);
		SERIALIZED_MEMBER_OPTIONAL(isLockedAngularZ, false);
		SERIALIZED_MEMBER_OPTIONAL(offset, glm::vec3{});
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