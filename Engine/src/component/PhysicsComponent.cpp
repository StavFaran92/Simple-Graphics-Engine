#include "PhysicsComponent.h"

#include "core/Engine.h"
#include "physics/PhysicsSystem.h"

void PhysicsComponent::addForce(glm::vec3 force)
{
	m_force += force;
	isChanged = true;
}

void PhysicsComponent::setForce(glm::vec3 force)
{
	m_force = glm::vec3(0);
	addForce(force);
}

void PhysicsComponent::move(glm::vec3 position)
{
	m_targetPisition = position;
	isChanged = true;
}

void PhysicsComponent::setRigidBodyType(RigidbodyType type)
{
	rigidBodyType = type;
}

void PhysicsComponent::setQueryEnabled(bool enabled)
{
	isQueryEnabled = enabled;
	Engine::get()->getPhysicsSystem()->applyQueryEnabled(*this);
}

CollisionShape PhysicsComponent::getShapeType() const
{
	Collider* c = collider.get();
	if (dynamic_cast<CollisionBox*>(c)) return CollisionShape::BOX;
	if (dynamic_cast<CollisionSphere*>(c)) return CollisionShape::SPHERE;
	if (dynamic_cast<CollisionCapsule*>(c)) return CollisionShape::CAPSULE;
	if (dynamic_cast<CollisionMesh*>(c)) return CollisionShape::MESH;
	if (dynamic_cast<CollisionTerrain*>(c)) return CollisionShape::TERRAIN;
	return CollisionShape::NONE;
}

void PhysicsComponent::createShape(CollisionShape type)
{
	switch (type)
	{
	case CollisionShape::BOX: collider = std::make_shared<CollisionBox>(); break;
	case CollisionShape::SPHERE: collider = std::make_shared<CollisionSphere>(); break;
	case CollisionShape::CAPSULE: collider = std::make_shared<CollisionCapsule>(); break;
	case CollisionShape::MESH: collider = std::make_shared<CollisionMesh>(); break;
	case CollisionShape::TERRAIN: collider = std::make_shared<CollisionTerrain>(); break;
	default: collider = nullptr; break;
	}
}
