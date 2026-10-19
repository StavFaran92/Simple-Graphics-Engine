#include "ComponentSnapshot.h"

#include "sge.h"
#include "serialize/Archiver.h"

#include <stdexcept>

Entity entityFromId(entity_id id)
{
	auto scene = Engine::get()->getContext()->getActiveScene();
	return Entity{ static_cast<entt::entity>(id), &scene->getRegistry() };
}

ComponentSnapshot takeSnapshot(Entity e, const std::string& component)
{
	ComponentSnapshot snapshot;
	snapshot.entity = e.handlerID();
	snapshot.component = component;
	snapshot.present = Archiver::saveComponent(e, component, snapshot.data);
	return snapshot;
}

std::vector<ComponentSnapshot> takeEntitySnapshot(Entity e)
{
	std::vector<ComponentSnapshot> snapshots;
	for (const auto& component : Archiver::getRegisteredComponentNames())
	{
		ComponentSnapshot snapshot = takeSnapshot(e, component);
		if (snapshot.present)
			snapshots.push_back(std::move(snapshot));
	}
	return snapshots;
}

void applySnapshot(const ComponentSnapshot& snapshot)
{
	Entity e = entityFromId(snapshot.entity);
	if (!e.valid())
		throw std::runtime_error("Entity " + std::to_string(snapshot.entity) + " no longer exists");

	if (snapshot.present)
		Archiver::loadComponent(e, snapshot.component, snapshot.data);
	else
		Archiver::removeComponent(e, snapshot.component);
}
