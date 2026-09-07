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
#include "core/Engine.h"
#include "component/Transformation.h"
#include "runtime/Context.h"

PrefabResourceRef Prefab::load(const std::string& fileLocation, PrefabLoadDescriptor desc)
{
	desc.sourcePath = fileLocation;
	std::string filepath = desc.sourcePath;
	std::ifstream is(filepath);
	cereal::JSONInputArchive iarchive(is);

	

	try
	{
		PrefabData data;
		iarchive(data);
		PrefabResourceRef prefab = Factory<Prefab>::create();
		prefab->m_data = data;
		return prefab;

	}
	catch (const cereal::Exception& e)
	{
		logError("Deserialization Error occured: {}", e.what());
	}

	return PrefabResourceRef::empty;
}

PrefabData Prefab::serializeEntityToPrefabData(const Entity& e)
{
	PrefabData prefabData;
	serializeEntityToPrefabDataHelper(e, prefabData);
	return prefabData;
}

void Prefab::serializeEntityToPrefabDataHelper(const Entity& e, PrefabData& prefabData)
{
	prefabData.m_serializedPrefab.push_back(Archiver::serializeEntity(e));
	auto& children = e.getComponent<Transformation>().getChildren();
	if (children.size() > 0)
	{
		for (auto& [_, child] : children)
		{
			serializeEntityToPrefabDataHelper(child, prefabData);
		}
	}
}

PrefabResourceRef Prefab::create(const Entity& e)
{
	PrefabResourceRef prefab = Factory<Prefab>::create();
	PrefabData prefabData = serializeEntityToPrefabData(e);
	prefab->m_data = prefabData;

	return prefab;
}

// ============================================================
//  PrefabAsset (Asset wrapper)
// ============================================================

void PrefabAsset::serialize(nlohmann::json& j) const
{
	// PrefabAsset does not add extra data on top of Prefab resource
	// at the moment. Use an explicit empty object placeholder so we
	// can extend this later if needed.
	j = nlohmann::json::object();
}

void PrefabAsset::deserialize(const nlohmann::json& j)
{
	// No prefab-asset specific fields to restore yet.
	(void)j;
}

Entity Prefab::Instansiate(glm::vec3 position/*= {}*/)
{
	std::map<entity_id, Entity> entityIDRemapTable;
	std::vector<Entity> createdEntities;

	for (SerializedEntity& serializedEntity : m_data.m_serializedPrefab)
	{
		auto scene = Engine::get()->getContext()->getActiveScene();

		auto& e = Archiver::deserializeEntity(serializedEntity, Engine::get()->getContext()->getActiveScene());

		for (auto& cbWrapper : ComponentSerdes::getRegistry())
		{
			cbWrapper.resolve(e, scene);
		}

		for (auto& cbWrapper : ComponentSerdes::getRegistry())
		{
			cbWrapper.postLoad(e, scene);
		}

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

	// The deserialized parent/children still reference the stale ids from the
	// serialized template. Local transforms were captured relative to the
	// original parent, and since the whole hierarchy is cloned together, those
	// local values are already correct relative to the remapped parent too -
	// so just relink the raw entity references, no transform math needed.
	std::vector<entity_id> oldParentIDs;
	oldParentIDs.reserve(createdEntities.size());
	for (Entity& e : createdEntities)
	{
		auto& transform = e.getComponent<Transformation>();
		oldParentIDs.push_back(transform.m_parent.handlerID());
		transform.m_children.clear();
		transform.m_parent = Entity::EmptyEntity;
	}

	for (size_t i = 0; i < createdEntities.size(); ++i)
	{
		entity_id oldParentID = oldParentIDs[i];
		if (oldParentID == entt::null)
		{
			continue;
		}

		auto it = entityIDRemapTable.find(oldParentID);
		if (it == entityIDRemapTable.end())
		{
			logWarning("Could not locate oldID {} and remap table", oldParentID);
			continue;
		}

		Entity& newParent = it->second;
		Entity& child = createdEntities[i];

		child.getComponent<Transformation>().m_parent = newParent;
		newParent.getComponent<Transformation>().m_children[child.handlerID()] = child;
	}

	// First entity is the root.
	Entity& root = createdEntities.front();
	root.getComponent<Transformation>().setLocalPosition(position);
	
	return root;
}

