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
		// Data extract
		if (params.name.empty())
		{
			aInfo.name = std::filesystem::path(fileLocation).filename().stem().string();
		}
		else
		{
			aInfo.name = params.name;
		}

		aInfo.aType = AssetType::PREFAB;

		const std::string relativeFilepath = "/" + aInfo.name + ".asset";
		aInfo.filePath = relativeFilepath;
		aInfo.origFilePath = fileLocation;
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
	Resource<Prefab> prefab;

	aInfo.aType = AssetType::PREFAB;

	if (!aInfo.name.empty())
	{
		prefab = Factory<Prefab>::createUsingCustomUUID(aInfo.name);
	}
	else
	{
		prefab = Factory<Prefab>::create();
		aInfo.name = prefab.getUID();
	}

	aInfo.uuid = prefab.getUID();
	aInfo.isTransient = aInfo.isTransient;

	extractChildrenRecursive(e, prefab);

	//prefab->m_serializedPrefab.push_back(Archiver::serializeEntity(e));
	//auto& children = e.getComponent<Transformation>().getChildren();
	//if (children.size() > 0)
	//{
	//	for (auto& child : children)
	//	{

	//	}
	//}

	AssetLoader<Prefab>::save(aInfo, prefab);

	prefab->m_assetInfo = aInfo;

	return prefab;
}

void Prefab::Instansiate()
{
	std::map<entity_id, Entity> entityIDRemapTable;

	for (SerializedEntity& serializedEntity : m_serializedPrefab)
	{
		auto& e = Archiver::deserializeEntity(serializedEntity, *Engine::get()->getContext()->getActiveScene());
		entity_id oldEntityID = e.getComponent<ObjectComponent>().e.handlerID();

		entityIDRemapTable[oldEntityID] = e;
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

	auto& e = Archiver::deserializeEntity(m_serializedPrefab, *Engine::get()->getContext()->getActiveScene());
	entity_id oldEntityID = e.getComponent<ObjectComponent>().e.handlerID();

	std::string newName = e.getComponent<ObjectComponent>().name;
	newName += "_copy";

	// Todo - validate name is not taken

	e.getComponent<ObjectComponent>().name = newName;
	e.getComponent<ObjectComponent>().e = e;
	//Transformation transform = e.getComponent<Transformation>().setEntity();


}
