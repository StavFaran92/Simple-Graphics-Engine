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

struct ComponentSerdesEntry
{
	std::string name;
	SnapshotSerializeFunc serialize;
	SnapshotDeserializeFunc deserialize;
	std::function<void(Entity e, SceneResourceRef&)> init;
	std::function<void(Entity e, SceneResourceRef&)> onInit;
	std::function<void(Entity e, SceneResourceRef&)> postLoad;
	std::function<void(Entity e, SceneResourceRef&)> remove;
	std::function<std::shared_ptr<Component>()> makeDefault; // Returns null when T has no default constructor
};

class ComponentSerdes
{
public:
	static std::vector<ComponentSerdesEntry>& getRegistry()
	{
		static std::vector<ComponentSerdesEntry> registry;
		return registry;
	}

	template<typename T>
	static void RegisterComponentSerializer()
	{
		ComponentSerdesEntry entry;

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

		entry.init = [](Entity e, SceneResourceRef& scene) {
			entt::entity entity = e.handler();
			if (scene->getRegistry().get().all_of<T>(entity))
			{
				auto& c = scene->getRegistry().get().get<T>(entity);

				c.init(scene);
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

		// Removing needs the concrete type, so unlike save/load it can't go through shared_ptr<Component>
		entry.remove = [](Entity e, SceneResourceRef& scene) {
			scene->getRegistry().get().remove<T>(e.handler());
			scene->makeDirty();
		};

		entry.makeDefault = []() -> std::shared_ptr<Component> {
			if constexpr (std::is_default_constructible_v<T>)
				return std::make_shared<T>();
			else
				return nullptr;
		};

		getRegistry().push_back(std::move(entry));
	}
};


