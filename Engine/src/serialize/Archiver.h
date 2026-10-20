#pragma once

#include <cereal/types/polymorphic.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/optional.hpp>
#include <cereal/archives/json.hpp>
#include <iostream>
#include <fstream>

#include "core/Core.h"
#include "core/Logger.h"
#include <glm/glm.hpp>
#include "entt/entt.hpp"

#include "component/Component.h"

#include "serialize/CerealHelpers.h"

class Scene;
class Entity;
class Context;

struct SerializedEntity
{
	entt::entity entity;
	std::vector<std::shared_ptr<Component>> components;

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(entity);
		SERIALIZED_MEMBER(components);
	}
};

struct SerializedScene
{
	std::vector<SerializedEntity> serializedEntities;
	entt::entity gameCamera;

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(serializedEntities);
		SERIALIZED_MEMBER_OPTIONAL(gameCamera);
	}

};

//struct SerializedContext
//{
//	std::map<int, SerializedScene> serializedScenes;
//	int activeScene = 0;
//
//	template <class Archive>
//	void serialize(Archive& archive) {
//		SERIALIZED_MEMBER(serializedScenes);
//		SERIALIZED_MEMBER(activeScene);
//	}
//};

class EngineAPI Archiver
{
public:
	inline static void save()
	{
		instance->m_serializeCallback();

		logInfo("Successfully Saved Project.");
	}

	inline static void load()
	{
		instance->m_deserializeCallback();

		logInfo("Successfully Loaded Project.");
	}

	inline static void registerSerializeFunction(std::function<void()> cb)
	{
		instance->m_serializeCallback = cb;
	}

	inline static void registerDeserializeFunction(std::function<void()> cb)
	{
		instance->m_deserializeCallback = cb;
	}

	static SerializedEntity serializeEntity(Entity e);

	// The entity's components as cereal JSON, same format as in .scene files (for editor tooling)
	static std::string serializeEntityToJson(Entity e);

	// Names of every component type that can be serialized
	static std::vector<std::string> getRegisteredComponentNames();

	// "struct PhysicsComponent" (entt type name) -> "PhysicsComponent"
	static std::string componentTypeName(const std::string& enttTypeName);

	// Component as JSON; false if the entity doesn't have it
	static bool saveComponent(Entity e, const std::string& component, std::string& outJson);

	// A default-constructed component in saveComponent's format; false if it has no default constructor
	static bool defaultComponent(const std::string& component, std::string& outJson);

	// Puts the component's data from saveComponent's JSON on the entity, without setting it up
	static void deserializeComponent(Entity e, const std::string& component, const std::string& json);

	// deserializeComponent, then init and postLoad for this component; for entities whose neighbors exist
	static void loadComponent(Entity e, const std::string& component, const std::string& json);

	// Second half of loading, once every entity involved is deserialized: init on all of them, then postLoad
	// on all of them. Components read other entities while they set up (a Transformation reads its parent
	// and children), so this can't run per entity as each one loads.
	static void initializeEntities(const std::vector<Entity>& entities, SceneResourceRef& scene);

	static void removeComponent(Entity e, const std::string& component);

	static Entity deserializeEntity(SerializedEntity serializedEnt, SceneResourceRef& scene);

	static SerializedScene serializeScene(SceneResourceRef& scene);

	static void deserializeScene(SerializedScene serializedScene, SceneResourceRef& scene);

	//static SerializedContext serializeContext(const Context* ctx);

	//static void deserializeContext(SerializedContext serializedContext, Context* ctx);

private:

	std::function<void()> m_serializeCallback;
	std::function<void()> m_deserializeCallback;

	static Archiver* instance;
};