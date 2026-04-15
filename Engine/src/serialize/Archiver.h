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
		SERIALIZED_MEMBER_OPTIONAL(gameCamera, entt::null);
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