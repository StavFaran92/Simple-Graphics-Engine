#pragma once

#include "memory/Asset.h"
#include "core/Core.h"
#include "runtime/Entity.h"
#include "serialize/Archiver.h"

struct PrefabData
{
	Entity entity;

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(entity);
	}
};

struct EngineAPI PrefabCreateDescriptor : public ResourceCreateDescriptor
{
	ResourceWrapper<Resource> createResource() override;

	PrefabData data;
};

struct EngineAPI PrefabLoadDescriptor : public ResourceLoadDescriptor
{
	ResourceWrapper<Resource> loadResource() override;
};

// Resource
class EngineAPI Prefab : public Resource
{
public:
	static ResourceWrapper<Prefab> load(const std::string& fileLocation, PrefabLoadDescriptor desc = {});
	static ResourceWrapper<Prefab> create(const Entity& e);

	Entity Instansiate(glm::vec3 position = glm::vec3{ 0.0f });

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(m_serializedPrefab);
	}

private:
	static void extractChildrenRecursive(const Entity& e, ResourceWrapper<Prefab>& prefab);
private:
	std::vector<SerializedEntity> m_serializedPrefab;
};

// Asset
class EngineAPI PrefabAsset : public Asset
{
public:
	using ResourceType = Prefab;

	using Asset::Asset;
};