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

	// Whether raycasts / overlaps can hit this body, applied to the live shapes immediately
	void setQueryEnabled(bool enabled);

	// The collider's shape, NONE when there is no collider
	CollisionShape getShapeType() const;

	// Replaces the collider with a default one of this shape; NONE removes it
	void createShape(CollisionShape type);

	Collider* getShape() const { return collider.get(); }

	// The collider as T, null when it's another shape
	template <class T>
	T* getShape() const { return dynamic_cast<T*>(collider.get()); }

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZE_COMPONENT_BASE;
		SERIALIZED_MEMBER_OPTIONAL(rigidBodyType);
		SERIALIZED_MEMBER_OPTIONAL(collisionType);
		SERIALIZED_MEMBER_OPTIONAL(mass);
		SERIALIZED_MEMBER_OPTIONAL(collider);
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
	float mass = 0;
	bool isChanged = false;
	bool isQueryEnabled = true;
	glm::vec3 m_targetPisition{ 0 };
	glm::vec3 m_force{ 0 };
	void* simulatedBody = nullptr;

	glm::vec3 offset{};

	glm::vec3 m_parentCachedPos{ 0 };

private:
	std::shared_ptr<Collider> collider;
};

REGISTER_COMPONENT(PhysicsComponent)
