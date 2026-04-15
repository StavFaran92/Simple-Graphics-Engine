#include "serialize/Archiver.h"

#include "runtime/Scene.h"

#include "component/ComponentSerializer.h"
#include "component/CameraComponent.h"
#include "component/MeshRendererComponent.h"
#include "component/RenderableComponent.h"
#include "component/Transformation.h"
#include "memory/BuiltInAssets.h"
#include "core/Window.h"


Archiver* Archiver::instance = new Archiver();

template<typename T>
std::optional<T> getComponentIfExistsOpt(const Entity& e)
{
	std::optional<T> c;
	if (e.HasComponent<T>())
	{
		c = e.getComponent<T>();
	}
	return c;
}

SerializedEntity Archiver::serializeEntity(Entity e)
{
	SerializedEntity serializedEntity;
	serializedEntity.entity = e.handler();

	for (auto& cbWrapper : ComponentSerdes::getRegistry())
	{
		if (auto c = cbWrapper.serialize(e))
		{
			serializedEntity.components.push_back(c);
		}
	}

	return serializedEntity;
}

Entity Archiver::deserializeEntity(SerializedEntity serializedEnt, SceneResourceRef& scene)
{
	auto e = scene->getRegistry().getRegistry().create(serializedEnt.entity);
	auto entityHandler = Entity(e, &scene->getRegistry());

	for (auto& cbWrapper : ComponentSerdes::getRegistry())
	{
		for (auto& c : serializedEnt.components)
		{
			cbWrapper.deserialize(c, entityHandler, scene);
		}
	}

	return entityHandler;

}

SerializedScene Archiver::serializeScene(SceneResourceRef& scene)
{
	SerializedScene serializedScene;

	scene->getRegistry().get().each([&](auto entity) {
		Entity e(entity, &scene->getRegistry());

		// We only serialize entities that have object component
		//if (e.HasComponent<ObjectComponent>())
		{
			SerializedEntity serializedEntity = serializeEntity(e);

			serializedScene.serializedEntities.push_back(serializedEntity);
		}
		});

	serializedScene.gameCamera = scene->getGameCamera().handler();

	return serializedScene;
}

void Archiver::deserializeScene(SerializedScene serializedScene, SceneResourceRef& scene)
{
	//scene.getRegistry().getRegistry().clear();

	for (auto& serializedEnt : serializedScene.serializedEntities)
	{
		deserializeEntity(serializedEnt, scene);
	}

	Entity gameCameraEntity(serializedScene.gameCamera, &scene->getRegistry());
	scene->setGameCamera(gameCameraEntity);
}