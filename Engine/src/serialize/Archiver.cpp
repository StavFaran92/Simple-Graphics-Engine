#include "serialize/Archiver.h"

#include "runtime/Scene.h"

#include "component/ComponentSerializer.h"
#include "component/CameraComponent.h"
#include "component/MeshRendererComponent.h"
#include "component/RenderableComponent.h"
#include "component/Transformation.h"
#include "memory/BuiltInAssets.h"
#include "core/Window.h"
#include "core/Engine.h"
#include "runtime/Context.h"
#include "core/Logger.h"

#include <sstream>
#include <cereal/archives/json.hpp>


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

	for (auto& cbWrapper : ComponentSerdes::getRegistry())
	{
		if (auto c = cbWrapper.serialize(e))
		{
			serializedEntity.components.push_back(c);
		}
	}

	return serializedEntity;
}

std::string Archiver::serializeEntityToJson(Entity e)
{
	std::stringstream ss;
	{
		// The archive only completes its output when destroyed
		cereal::JSONOutputArchive archive(ss, cereal::JSONOutputArchive::Options::NoIndent());
		archive(cereal::make_nvp("entity", serializeEntity(e)));
	}
	return ss.str();
}

namespace
{
	const ComponentSerdesEntry& findEntry(const std::string& component)
	{
		for (auto& entry : ComponentSerdes::getRegistry())
		{
			if (Archiver::componentTypeName(entry.name) == component)
				return entry;
		}
		throw std::invalid_argument("Unknown component '" + component + "'");
	}

	// Going through cereal copies what the component points to, so the JSON doesn't change with later edits
	std::string componentToJson(const std::shared_ptr<Component>& c, const std::string& component)
	{
		std::stringstream ss;
		try
		{
			cereal::JSONOutputArchive archive(ss, cereal::JSONOutputArchive::Options::NoIndent());
			archive(cereal::make_nvp("component", c));
		}
		catch (const cereal::Exception& ex)
		{
			// Rethrown, not returned as false: false means "absent", and undo would remove the component
			logError("Saving component {} failed: {}", component, ex.what());
			throw std::runtime_error("Saving component " + component + " failed: " + ex.what());
		}
		return ss.str();
	}
}

std::string Archiver::componentTypeName(const std::string& enttTypeName)
{
	// entt type names on MSVC carry the "struct " / "class " keyword
	for (const char* prefix : { "struct ", "class " })
	{
		if (enttTypeName.rfind(prefix, 0) == 0)
			return enttTypeName.substr(strlen(prefix));
	}
	return enttTypeName;
}

std::vector<std::string> Archiver::getRegisteredComponentNames()
{
	std::vector<std::string> names;
	for (auto& entry : ComponentSerdes::getRegistry())
		names.push_back(componentTypeName(entry.name));
	return names;
}

bool Archiver::saveComponent(Entity e, const std::string& component, std::string& outJson)
{
	std::shared_ptr<Component> copy = findEntry(component).serialize(e);
	if (!copy)
		return false;

	outJson = componentToJson(copy, component);
	return true;
}

bool Archiver::defaultComponent(const std::string& component, std::string& outJson)
{
	std::shared_ptr<Component> c = findEntry(component).makeDefault();
	if (!c)
		return false;

	outJson = componentToJson(c, component);
	return true;
}

void Archiver::deserializeComponent(Entity e, const std::string& component, const std::string& json)
{
	const ComponentSerdesEntry& entry = findEntry(component);

	std::shared_ptr<Component> loaded;
	try
	{
		std::stringstream ss(json);
		cereal::JSONInputArchive archive(ss);
		archive(cereal::make_nvp("component", loaded));
	}
	catch (const cereal::Exception& ex)
	{
		logError("Loading component {} failed: {}", component, ex.what());
		throw std::runtime_error("Loading component " + component + " failed: " + ex.what());
	}

	auto scene = Engine::get()->getContext()->getActiveScene();
	entry.deserialize(loaded, e, scene);
}

void Archiver::loadComponent(Entity e, const std::string& component, const std::string& json)
{
	deserializeComponent(e, component, json);

	auto scene = Engine::get()->getContext()->getActiveScene();
	const ComponentSerdesEntry& entry = findEntry(component);
	entry.init(e, scene);
	entry.postLoad(e, scene);
}

void Archiver::initializeEntities(const std::vector<Entity>& entities, SceneResourceRef& scene)
{
	for (auto& entry : ComponentSerdes::getRegistry())
	{
		for (const Entity& e : entities)
			entry.init(e, scene);
	}

	for (auto& entry : ComponentSerdes::getRegistry())
	{
		for (const Entity& e : entities)
			entry.postLoad(e, scene);
	}
}

void Archiver::removeComponent(Entity e, const std::string& component)
{
	auto scene = Engine::get()->getContext()->getActiveScene();
	findEntry(component).remove(e, scene);
}

Entity Archiver::deserializeEntity(SerializedEntity serializedEnt, SceneResourceRef& scene)
{
	auto e = scene->getRegistry().getRegistry().create(serializedEnt.entity);
	auto entityHandler = Entity(e, &scene->getRegistry());

	for (auto& cbWrapper : ComponentSerdes::getRegistry())
	{
		for (auto& c : serializedEnt.components)
		{
			cbWrapper.deserialize(c, entityHandler, scene);
		}
	}

	return entityHandler;

}

SerializedScene Archiver::serializeScene(SceneResourceRef& scene)
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

void Archiver::deserializeScene(SerializedScene serializedScene, SceneResourceRef& scene)
{
	//scene.getRegistry().getRegistry().clear();

	for (auto& serializedEnt : serializedScene.serializedEntities)
	{
		deserializeEntity(serializedEnt, scene);
	}

	Entity gameCameraEntity(serializedScene.gameCamera, &scene->getRegistry());
	scene->setGameCamera(gameCameraEntity);
}