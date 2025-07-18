#pragma once

#include <cereal/types/polymorphic.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/optional.hpp>
#include <cereal/archives/json.hpp>
#include <iostream>
#include <fstream>

#include "Core.h"
#include "Logger.h"
#include <glm/glm.hpp>
#include "entt/entt.hpp"

#include "Component.h"
#include "Transformation.h"
#include "ScriptableEntity.h"
#include "Material.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "Skybox.h"
#include "Context.h"
#include "Animator.h"
#include "Terrain.h"

#include "serialize/CerealHelpers.h"

class Scene;
class Entity;
class Context;

namespace glm
{
	template<class Archive>
	void serialize(Archive& archive, glm::vec3& v) {
		archive(v.x, v.y, v.z);
	}

	template<class Archive>
	void serialize(Archive& archive, glm::vec2& v) {
		archive(v.x, v.y);
	}

	template<class Archive>
	void serialize(Archive& archive, glm::quat& q) {
		archive(q.x, q.y, q.z, q.w);
	}
};

template<typename T>
std::optional<T> getComponentIfExists(const Entity& e)
{
	std::optional<T> c;
	if (e.HasComponent<T>())
	{
		c = e.getComponent<T>();
	}
	return c;
}

struct SerializedEntity
{
	entt::entity entity;
	std::optional<Transformation> transform;
	std::optional<PhysicsComponent> physics;
	std::optional<PlayerController> playerController;
	std::optional<MeshComponent> mesh;
	std::optional<RenderableComponent> renderableComponent;
	std::optional<CameraComponent> camera;
	std::optional<NativeScriptComponent> nsc;
	std::optional<MaterialComponent> mat;
	std::optional<DirectionalLight> dLight;
	std::optional<PointLight> pLight;
	std::optional<ObjectComponent> obj;
	std::optional<SkyboxComponent> skybox;
	std::optional<ImageComponent> image;
	std::optional<Animator> animator;
	std::optional<Terrain> terrain;
	std::optional<TestComp> testComponent;
	std::optional<ShaderComponent> shader;

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(entity),
		SERIALIZED_MEMBER(transform),
		SERIALIZED_MEMBER(physics),
		SERIALIZED_MEMBER(playerController),
		SERIALIZED_MEMBER(mesh),
		SERIALIZED_MEMBER(renderableComponent),
		SERIALIZED_MEMBER(camera),
		SERIALIZED_MEMBER(nsc),
		SERIALIZED_MEMBER(mat),
		SERIALIZED_MEMBER(dLight),
		SERIALIZED_MEMBER(pLight),
		SERIALIZED_MEMBER(obj),
		SERIALIZED_MEMBER(skybox),
		SERIALIZED_MEMBER(image),
		SERIALIZED_MEMBER(animator),
		SERIALIZED_MEMBER(terrain),
		SERIALIZED_MEMBER(testComponent),
		SERIALIZED_MEMBER(shader);
	}
};

struct SerializedScene
{
	std::vector<SerializedEntity> serializedEntities;
	entt::entity gameCamera;

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(serializedEntities),
		SERIALIZED_MEMBER_OPTIONAL(gameCamera, 0);
	}

};

struct SerializedContext
{
	std::map<int, SerializedScene> serializedScenes;
	int activeScene = 0;

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(serializedScenes),
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