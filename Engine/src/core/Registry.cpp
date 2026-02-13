#include "core/Registry.h"

#include "runtime/Entity.h"
#include "component/Component.h"
#include "component/Transformation.h"
#include "component/ObjectComponent.h"
#include "core/Logger.h"


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
	entt::entity e = m_registry.create();
	auto entityHandler = Entity(e, this);
	entityHandler.addComponent<Transformation>(entityHandler);
	return entityHandler;
}

void SGE_Regsitry::removeEntity(const Entity& e)
{
	auto id = e.handlerID();
	m_registry.destroy(e.handler());

	logDebug("Removed entity: " + std::to_string(id));
}
