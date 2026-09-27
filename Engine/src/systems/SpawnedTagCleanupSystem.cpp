#include "systems/SpawnedTagCleanupSystem.h"

#include "component/EngineComponents.h"
#include "runtime/Scene.h"

SpawnedTagCleanupSystem::SpawnedTagCleanupSystem()
{
}

void SpawnedTagCleanupSystem::update(Scene* scene)
{
	scene->getRegistry().getRegistry().clear<JustSpawned>();
}
