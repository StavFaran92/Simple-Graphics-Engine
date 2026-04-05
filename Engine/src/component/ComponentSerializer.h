#pragma once

#include "runtime/Entity.h"
#include "component/Component.h"

class EngineAPI ComponentSerializer
{
public:
	using SerializeFn = std::function<std::shared_ptr<Component>(const Entity& e)>;
	using DeserializeFn = std::function<void(std::shared_ptr<Component>, Entity, ResourceWrapper<Scene>&)>;

	static void registerSerializeFunc(const SerializeFn& fn);
	static void serializeComponents(const Entity& e, std::vector<std::shared_ptr<Component>>& components);

	static void registerDeserializeFunc(const DeserializeFn& fn);
	static void deserializeComponents(const std::vector<std::shared_ptr<Component>>& components, Entity& e, ResourceWrapper<Scene>& scene);

private:

	// Magic static
	static std::vector<SerializeFn>& getSerializeFunctionRegistry() {
		static std::vector<SerializeFn> serializeFunctionRegistry;
		return serializeFunctionRegistry;
	}

	static std::vector<DeserializeFn>& getDeserializeFunctionRegistry() {
		static std::vector<DeserializeFn> deserializeFunctionRegistry;
		return deserializeFunctionRegistry;
	}

private:
	
};


// The above 2 can be merged into one 
// (Even a more abstract tool could be made if combined with asset registerer [maybe too abstract to be useful])

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

#include "cereal/archives/json.hpp"
#include "serialize/CerealHelpers.h"

using SnapshotSerializeFunc = std::function<void(entt::snapshot&, cereal::JSONOutputArchive&)>;
using SnapshotSerializeFunc_2 = std::function<std::shared_ptr<Component>(const Entity& e)>;
using SnapshotDeserializeFunc = std::function<void(entt::snapshot_loader&, cereal::JSONInputArchive&)>;
using SnapshotDeserializeFunc_2 = std::function<void(std::shared_ptr<Component> c, Entity entityHandler, ResourceWrapper<Scene>& scene)>;

struct SerializerEntry
{
	std::string name;
	SnapshotSerializeFunc serialize;
	SnapshotSerializeFunc_2 serialize_2;
	SnapshotDeserializeFunc deserialize;
	SnapshotDeserializeFunc_2 deserialize_2;
	std::function<void(ResourceWrapper<Scene>&)> resolve;
	std::function<void(ResourceWrapper<Scene>&)> postLoad;
};

#include "runtime/Scene.h"

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

		entry.serialize = [](entt::snapshot& snapshot, cereal::JSONOutputArchive& output)
			{
				snapshot.component<T>(output);
			};

		entry.serialize_2 = [](const Entity& e)
			{
				std::shared_ptr<Component> c;
				if (e.HasComponent<T>())
				{
					c = std::make_shared<T>(e.getComponent<T>());
				}
				return c;
			};

		entry.deserialize = [](entt::snapshot_loader& snapshot, cereal::JSONInputArchive& input)
			{
				snapshot.component<T>(input);
			};

		entry.deserialize_2 = [](std::shared_ptr<Component> c, Entity entityHandler, ResourceWrapper<Scene>& scene)
			{
				if (auto tc = std::dynamic_pointer_cast<T>(c))
				{
					scene->getRegistry().get().emplace_or_replace<T>(entityHandler.handler(), *tc);
					//entityHandler.addComponent<T>(*tc);
				}
			};

		entry.resolve = [](ResourceWrapper<Scene>& scene) { 
			for(auto& [entity, c] : scene->getRegistry().get().view<T>().each())
			{
				c.resolve(scene);
				c.registerDependencyListener([scene](UUID uid) { scene.get()->makeDirty(); });
			}
		};

		entry.postLoad = [](ResourceWrapper<Scene>& scene) {
			for (auto& [entity, c] : scene->getRegistry().get().view<T>().each())
			{
				c.postLoad(scene);
			}
		};

		getRegistry().push_back(std::move(entry));
	}
};


