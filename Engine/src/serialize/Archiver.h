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
#include "component/Transformation.h"
#include "component/ScriptableEntity.h"
#include "render/Material.h"
#include "lights/DirectionalLight.h"
#include "lights/PointLight.h"
#include "systems/Skybox.h"
#include "runtime/Context.h"
#include "animation/Animator.h"
#include "component/Terrain.h"
#include "component/FoliageComponent.h"

#include "serialize/CerealHelpers.h"

class Scene;
class Entity;
class Context;

namespace glm
{
	template<class Archive>
	void serialize(Archive& archive, glm::vec3& v) {
			SERIALIZED_MEMBER(v.x); 
			SERIALIZED_MEMBER(v.y); 
			SERIALIZED_MEMBER(v.z);
	}

	template<class Archive>
	void serialize(Archive& archive, glm::vec2& v) {
		SERIALIZED_MEMBER(v.x); 
		SERIALIZED_MEMBER(v.y);
	}

	template<class Archive>
	void serialize(Archive& archive, glm::quat& q) {
			SERIALIZED_MEMBER(q.x); 
			SERIALIZED_MEMBER(q.y); 
			SERIALIZED_MEMBER(q.z); 
			SERIALIZED_MEMBER(q.w);
	}
};

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

struct SerializedContext
{
	std::map<int, SerializedScene> serializedScenes;
	int activeScene = 0;

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(serializedScenes);
		SERIALIZED_MEMBER(activeScene);
	}
};

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

	static void deserializeEntity(SerializedEntity serializedEnt, Scene& scene);

	static SerializedScene serializeScene(Scene* scene);

	static void deserializeScene(SerializedScene serializedScene, Scene& scene);

	static SerializedContext serializeContext(const Context* ctx);

	static void deserializeContext(SerializedContext serializedContext, Context* ctx);

private:

	std::function<void()> m_serializeCallback;
	std::function<void()> m_deserializeCallback;

	static Archiver* instance;
};