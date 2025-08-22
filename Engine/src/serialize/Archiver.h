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

template<typename T>
std::shared_ptr<T> getComponentIfExists2(const Entity& e)
{
	std::shared_ptr<T> c;
	if (e.HasComponent<T>())
	{
		c = std::shared_ptr<T>(e.tryGetComponent<T>(), [](T*) {});
	}
	return c;
}

struct SerializedEntity
{
	entt::entity entity;
	std::vector<std::shared_ptr<Component>> components;

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
    //std::optional<TestComp> testComponent;
    std::optional<ShaderComponent> shader;
    std::optional<FoliageComponent> foliage;

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(entity);
		SERIALIZED_MEMBER(components);

		SERIALIZED_MEMBER(transform);
		SERIALIZED_MEMBER(physics);
		SERIALIZED_MEMBER(playerController);
		SERIALIZED_MEMBER(mesh);
		SERIALIZED_MEMBER(renderableComponent);
		SERIALIZED_MEMBER(camera);
		SERIALIZED_MEMBER(nsc);
		SERIALIZED_MEMBER(mat);
		SERIALIZED_MEMBER(dLight);
		SERIALIZED_MEMBER(pLight);
		SERIALIZED_MEMBER(obj);
		SERIALIZED_MEMBER(skybox);
		SERIALIZED_MEMBER(image);
        SERIALIZED_MEMBER(animator);
        SERIALIZED_MEMBER(terrain);
        //SERIALIZED_MEMBER(testComponent);
        SERIALIZED_MEMBER(shader);
		SERIALIZED_MEMBER_OPTIONAL(foliage, {});
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