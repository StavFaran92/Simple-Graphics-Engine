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
using SnapshotDeserializeFunc = std::function<void(entt::snapshot_loader&, cereal::JSONInputArchive&)>;

struct SerializerEntry
{
	std::string name;
	SnapshotSerializeFunc serialize;
	SnapshotDeserializeFunc deserialize;
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

		entry.deserialize = [](entt::snapshot_loader& snapshot, cereal::JSONInputArchive& input)
			{
				snapshot.component<T>(input);
			};

		entry.postLoad = [](ResourceWrapper<Scene>& scene) { 
			for(auto& [entity, c] : scene->getRegistry().get().view<T>().each())
			{
				c.resolve(scene);
			}
		};

		getRegistry().push_back(std::move(entry));
	}
};

