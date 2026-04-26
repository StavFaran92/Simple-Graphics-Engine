#pragma once

#include "runtime/Entity.h"
#include "component/Component.h"
#include "runtime/Scene.h"

// Serialize
template<typename T>
class ComponentSerializeFnRegister
{
public:
	static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");

	ComponentSerializeFnRegister()
	{
		ComponentSerdes::RegisterComponentSerializer<T>();
	}

	static ComponentSerializeFnRegister<T> staticRegister;
};

template<typename T>
ComponentSerializeFnRegister<T> ComponentSerializeFnRegister<T>::staticRegister;

using SnapshotSerializeFunc = std::function<std::shared_ptr<Component>(const Entity& e)>;
using SnapshotDeserializeFunc = std::function<void(std::shared_ptr<Component> c, Entity entityHandler, SceneResourceRef& scene)>;

struct SerializerEntry
{
	std::string name;
	SnapshotSerializeFunc serialize;
	SnapshotDeserializeFunc deserialize;
	std::function<void(Entity e, SceneResourceRef&)> resolve;
	std::function<void(Entity e, SceneResourceRef&)> onInit;
	std::function<void(Entity e, SceneResourceRef&)> postLoad;
};

class ComponentSerdes
{
public:
	static std::vector<SerializerEntry>& getRegistry()
	{
		static std::vector<SerializerEntry> registry;
		return registry;
	}

	template<typename T>
	static void RegisterComponentSerializer()
	{
		SerializerEntry entry;

		entry.name = entt::type_name<T>().value(); // quick + dirty

		entry.serialize = [](const Entity& e)
			{
				std::shared_ptr<Component> c;
				if (e.HasComponent<T>())
				{
					c = std::make_shared<T>(e.getComponent<T>());
				}
				return c;
			};

		entry.deserialize = [](std::shared_ptr<Component> c, Entity entityHandler, SceneResourceRef& scene)
			{
				if (auto tc = std::dynamic_pointer_cast<T>(c))
				{
					scene->getRegistry().get().emplace_or_replace<T>(entityHandler.handler(), *tc);
					scene->makeDirty();
				}
			};

		entry.resolve = [](Entity e, SceneResourceRef& scene) { 
			entt::entity entity = e.handler();
			if (scene->getRegistry().get().all_of<T>(entity))
			{
				auto& c = scene->getRegistry().get().get<T>(entity);

				c.resolve(scene);
			}
		};

		entry.onInit = [](Entity e, SceneResourceRef& scene) {
			entt::entity entity = e.handler();
			if (scene->getRegistry().get().all_of<T>(entity))
			{
				auto& c = scene->getRegistry().get().get<T>(entity);

				c.onInit(scene);
			}
		};

		entry.postLoad = [](Entity e, SceneResourceRef& scene) {
			entt::entity entity = e.handler();
			if (scene->getRegistry().get().all_of<T>(entity))
			{
				auto& c = scene->getRegistry().get().get<T>(entity);

				c.postLoad(scene);
			}
		};

		getRegistry().push_back(std::move(entry));
	}
};


