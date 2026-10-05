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
