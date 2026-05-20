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
	void setRigidBodyType(RigidbodyType type);

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZE_COMPONENT_BASE;
		SERIALIZED_MEMBER_OPTIONAL(rigidBodyType);
		SERIALIZED_MEMBER_OPTIONAL(collisionType);
		SERIALIZED_MEMBER_OPTIONAL(mass);
		SERIALIZED_MEMBER_OPTIONAL(collider);
		SERIALIZED_MEMBER_OPTIONAL(shapeType);
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
	RigidbodyType rigidBodyType = RigidbodyType::Static;
	CollisionType collisionType = CollisionType::Collider;
	CollisionShape shapeType = CollisionShape::NONE;
	std::shared_ptr<Collider> collider;
	float mass = 0;
	bool isChanged = false;
	glm::vec3 m_targetPisition{ 0 };
	glm::vec3 m_force{ 0 };
	void* simulatedBody = nullptr;

	glm::vec3 offset{};

	glm::vec3 m_parentCachedPos{ 0 };
};

REGISTER_COMPONENT(PhysicsComponent)