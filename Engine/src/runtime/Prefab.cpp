#include "Prefab.h"

#include <fstream>
#include <filesystem>

#include <cereal/types/polymorphic.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/optional.hpp>
#include <cereal/archives/json.hpp>

#include "memory/AssetLoader.h"
#include "core/Factory.h"
#include "component/ObjectComponent.h"

namespace {
	struct PrefabManagerRegistration {
		PrefabManagerRegistration() {
			AssetFactory::registerManager(AssetType::PREFAB, std::make_shared<PrefabAssetManager>());
		}
	} _PrefabManagerRegistration;
}

bool PrefabAssetManager::copyFiles(const std::string& fileLocation, AssetInfo& aInfo)
{
	return false;
}

ResourceWrapper<ResourceBase> PrefabAssetManager::load(AssetInfo& aInfo)
{
	auto projectDir = Engine::get()->getProjectDirectory();
	std::ifstream is(projectDir + aInfo.filePath);
	cereal::JSONInputArchive iarchive(is);
	Prefab* loadedPrefab = new Prefab();

	try
	{
		iarchive(*loadedPrefab);
		Engine::get()->getMemoryPool().add(aInfo.uuid, loadedPrefab);
		return ResourceWrapper<Prefab>(aInfo.uuid);

	}
	catch (const cereal::Exception& e)
	{
		logError("Deserialization Error occured: {}", e.what());
	}

	return ResourceWrapper<Prefab>::empty;
}

void PrefabAssetManager::save(const ResourceWrapper<ResourceBase>& prefab, const AssetInfo& aInfo)
{
	auto projectDir = Engine::get()->getProjectDirectory();
	std::ofstream os(projectDir + "/" + aInfo.filePath);
	cereal::JSONOutputArchive oarchive(os);

	try
	{
		oarchive(*prefab.as<Prefab>().get());
	}
	catch (const cereal::Exception& e)
	{
		logError("Serialization Error occured: {}", e.what());
	}
}

ResourceWrapper<Prefab> Prefab::import(const std::string& fileLocation, PrefabImportSettings desc)
{
	desc.aType = AssetType::PREFAB;
	desc.origFilePath = fileLocation;
	return Engine::get()->getSubSystem<Assets>()->importAsset(fileLocation, desc).as<Prefab>();
}

void Prefab::save(const ResourceWrapper<Prefab>& prefab, AssetInfo aInfo)
{
	//Engine::get()->getSubSystem<Assets>()->updateAsset<Prefab>(prefab, aInfo); // todo fix
}

void Prefab::extractChildrenRecursive(const Entity& e, ResourceWrapper<Prefab>& prefab)
{
	prefab->m_serializedPrefab.push_back(Archiver::serializeEntity(e));
	auto& children = e.getComponent<Transformation>().getChildren();
	if (children.size() > 0)
	{
		for (auto& [_, child] : children)
		{
			extractChildrenRecursive(child, prefab);
		}
	}
}

ResourceWrapper<Prefab> Prefab::create(const Entity& e)
{
	//AssetInfo aInfo(aDesc);

	//aInfo.aType = AssetType::PREFAB;
	//aInfo.ext = ".asset";

	ResourceWrapper<Prefab> prefab = Factory<Prefab>::create();

	extractChildrenRecursive(e, prefab);

	//AssetLoader<Prefab>::save(prefab, aInfo.parse());

	return prefab;
}

Entity Prefab::Instansiate(glm::vec3 position/*= {}*/)
{
	std::map<entity_id, Entity> entityIDRemapTable;
	std::vector<Entity> createdEntities;

	for (SerializedEntity& serializedEntity : m_serializedPrefab)
	{
		auto& e = Archiver::deserializeEntity(serializedEntity, *Engine::get()->getContext()->getActiveScene());
		entity_id oldEntityID = e.getComponent<ObjectComponent>().e.handlerID();

		entityIDRemapTable[oldEntityID] = e;

		std::string newName = e.getComponent<ObjectComponent>().name;
		newName += "_copy";

		// Todo - validate name is not taken

		e.getComponent<ObjectComponent>().name = newName;
		e.getComponent<ObjectComponent>().e = e;
		e.getComponent<Transformation>().entity = e;

		createdEntities.push_back(e);
	}

	// for each entity
	// deserealize and obtain new id
	// add to table of old to new id
	// change id in object comp
	// change name in object comp
	// change entity id in in transform
	// change root id in transform


	// for each entity
		// in transform iterate children
			// give child new id using generated table

	for (Entity& e : createdEntities)
	{
		auto& transform = e.getComponent<Transformation>();
		auto& children = transform.getChildren();
		for (int i=0; i< children.size(); i++)
		{
			entity_id oldID = children[i].handlerID();
			auto it = entityIDRemapTable.find(oldID);
			if (it == entityIDRemapTable.end()) 
			{
				logWarning("Could not locate oldID {} and remap table", oldID);
				continue;
			}
			children[i] = it->second;
		}

		entity_id oldParentID = transform.m_parent.handlerID();
		if (oldParentID == entt::null)
		{
			e.getComponent<Transformation>().m_parent = Entity::EmptyEntity;
		}
		else
		{
			auto it = entityIDRemapTable.find(oldParentID);
			if (it == entityIDRemapTable.end())
			{
				logWarning("Could not locate oldID {} and remap table", oldParentID);
				continue;
			}
			e.getComponent<Transformation>().m_parent = it->second;
		}
	}

	// First entity is the root.
	Entity& root = createdEntities.front();
	root.getComponent<Transformation>().setLocalPosition(position);
	
	return root;
}


