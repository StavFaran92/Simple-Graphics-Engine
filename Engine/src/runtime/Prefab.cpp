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

template<>
struct AssetTraits<Prefab>
{
	static bool copyFiles(const std::string& fileLocation, AssetInfo& aInfo)
	{
		return false;
	}

	static void convertAssetLoadParamsToAssetInfo(const std::string& fileLocation, const BaseAssetParameters& params, AssetInfo& aInfo)
	{
		aInfo.aType = AssetType::PREFAB;
		aInfo.fileName = aInfo.name + ".asset";;
	}

	static Resource<Prefab> load(AssetInfo& aInfo)
	{
		auto projectDir = Engine::get()->getProjectDirectory();
		std::ifstream is(projectDir + aInfo.filePath);
		cereal::JSONInputArchive iarchive(is);
		Prefab* loadedPrefab = new Prefab();

		try
		{
			iarchive(*loadedPrefab);
			Engine::get()->getMemoryPool().add(aInfo.uuid, loadedPrefab);
			return Resource<Prefab>(aInfo.uuid);

		}
		catch (const cereal::Exception& e)
		{
			logError("Deserialization Error occured: {}", e.what());
		}

		return Resource<Prefab>::empty;
	}

	static void save(AssetInfo& aInfo, const Resource<Prefab>& prefab)
	{
		auto projectDir = Engine::get()->getProjectDirectory();
		std::ofstream os(projectDir + aInfo.filePath);
		cereal::JSONOutputArchive oarchive(os);

		try
		{
			oarchive(*prefab.get());
		}
		catch (const cereal::Exception& e)
		{
			logError("Serialization Error occured: {}", e.what());
		}
	}
};

static AssetFnRegister<AssetType::PREFAB> assetRegister(AssetTraits<Prefab>::load);

Resource<Prefab> Prefab::import(const std::string& fileLocation, const PrefabImportSettings& settings)
{
	return AssetLoader<Prefab>::import(fileLocation, settings);
}

Resource<Prefab> Prefab::loadTransient(const std::string& fileLocation, const PrefabImportSettings& settings)
{
	return AssetLoader<Prefab>::loadTransient(fileLocation, settings);
}

void Prefab::extractChildrenRecursive(const Entity& e, Resource<Prefab>& prefab)
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

Resource<Prefab> Prefab::create(const Entity& e, AssetInfo& aInfo)
{
	aInfo.aType = AssetType::PREFAB;
	aInfo.ext = ".asset";

	Resource<Prefab> prefab = AssetLoader<Prefab>::create(aInfo);

	extractChildrenRecursive(e, prefab);

	AssetLoader<Prefab>::save(aInfo, prefab);

	return prefab;
}

void Prefab::Instansiate()
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



	

	


}
