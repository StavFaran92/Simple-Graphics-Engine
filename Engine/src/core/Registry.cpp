#include "Registry.h"

#include "Entity.h"
#include "Component.h"
#include "Transformation.h"


entt::registry& SGE_Regsitry::get()
{
	return m_registry;
}

entt::registry& SGE_Regsitry::getRegistry()
{
	return m_registry;
}

const entt::registry& SGE_Regsitry::getRegistry() const
{
	return m_registry;
}

Entity SGE_Regsitry::createEntity()
{
	std::string name = "temp";
	return createEntity(name);
}

Entity SGE_Regsitry::createEntity(const std::string& name)
{
	entt::entity e = m_registry.create();
	auto entityHandler = Entity(e, this);
	entityHandler.addComponent<Transformation>(entityHandler);
	entityHandler.addComponent<ObjectComponent>(entityHandler, name); //todo fix
	return entityHandler;
}

void SGE_Regsitry::removeEntity(const Entity& e)
{
	auto id = e.handlerID();
	m_registry.destroy(e.handler());

	logDebug("Removed entity: " + std::to_string(id));
}
