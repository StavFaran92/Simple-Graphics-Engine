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

	ComponentSerializer::serializeComponents(e, serializedEntity.components);

	return serializedEntity;
}

Entity Archiver::deserializeEntity(SerializedEntity serializedEnt, ResourceWrapper<Scene>& scene)
{
	auto e = scene->getRegistry().getRegistry().create(serializedEnt.entity);
	auto entityHandler = Entity(e, &scene->getRegistry());

	ComponentSerializer::deserializeComponents(serializedEnt.components, entityHandler, scene);

	return entityHandler;

}

SerializedScene Archiver::serializeScene(ResourceWrapper<Scene>& scene)
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

void Archiver::deserializeScene(SerializedScene serializedScene, ResourceWrapper<Scene>& scene)
{
	//scene.getRegistry().getRegistry().clear();

	for (auto& serializedEnt : serializedScene.serializedEntities)
	{
		deserializeEntity(serializedEnt, scene);
	}

	Entity gameCameraEntity(serializedScene.gameCamera, &scene->getRegistry());
	if (!gameCameraEntity.valid() || !gameCameraEntity.HasComponent<CameraComponent>())
	{
		logError("Invalid game camera serialized, creating a new camera.");

		gameCameraEntity = scene->createEntity("Main Camera");
		gameCameraEntity.addComponent<CameraComponent>(CameraComponent::createPerspectiveCamera(45.0f, (float)Engine::get()->getWindow()->getWidth() / Engine::get()->getWindow()->getHeight(), 0.1f, 1000.0f));
		gameCameraEntity.getComponent<Transformation>().setLocalPosition({ 10,10,10 });
		gameCameraEntity.getComponent<CameraComponent>().center = { 0,0,0 };
		gameCameraEntity.getComponent<CameraComponent>().up = { 0,1,0 };
		gameCameraEntity.addComponent<MeshRendererComponent>().mesh = BuiltInAssets::getByName<ModelAsset>(SGE_MESH_CAMERA);
		gameCameraEntity.addComponent<RenderableComponent>();
	}
	scene->setGameCamera(gameCameraEntity);

	// We postpone the transform update because at the moment of transform creation not all transforms 
	// have been created yet.
	for (auto& [e, trans] : scene->getRegistry().getRegistry().view<Transformation>().each())
	{
		trans.forceUpdate();
	}

	
}

//SerializedContext Archiver::serializeContext(const Context* ctx)
//{
//	SerializedContext serializedContext;
//
//	for (auto& [sceneID, scene] : ctx->getAllScenes())
//	{
//		serializedContext.serializedScenes[sceneID] = serializeScene(scene.get());
//	}
//
//	serializedContext.activeScene = ctx->getActiveSceneID();
//
//	return serializedContext;
//}
//
//void Archiver::deserializeContext(SerializedContext serializedContext, Context* ctx)
//{
//	ctx->m_scenes.clear();
//
//	ctx->m_activeScene = serializedContext.activeScene;
//
//	for (auto& [sceneID, serializedScene] : serializedContext.serializedScenes)
//	{
//		std::shared_ptr<Scene> scene = std::make_shared<Scene>(ctx);
//		ctx->m_scenes[sceneID] = scene;
//		deserializeScene(serializedScene, *scene.get());
//	}
//
//	
//}
