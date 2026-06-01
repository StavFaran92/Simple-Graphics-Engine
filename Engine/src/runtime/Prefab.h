#pragma once

#include "memory/Asset.h"
#include "memory/AssetAliases.h"
#include "core/Core.h"
#include "runtime/Entity.h"
#include "serialize/Archiver.h"

struct PrefabData
{
	std::vector<SerializedEntity> m_serializedPrefab;

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(m_serializedPrefab);
	}
};

struct EngineAPI PrefabCreateDescriptor : public ResourceBuildDescriptor
{
	PrefabData data;
};

struct EngineAPI PrefabLoadDescriptor : public ResourceLoadDescriptor
{
};

// Resource
class EngineAPI Prefab : public Resource
{
public:
	static PrefabResourceRef load(const std::string& fileLocation, PrefabLoadDescriptor desc = {});
	static PrefabResourceRef create(const Entity& e);

	Entity Instansiate(glm::vec3 position = glm::vec3{ 0.0f });

	static PrefabData serializeEntityToPrefabData(const Entity& e); // todo make private
private:
	static void serializeEntityToPrefabDataHelper(const Entity& e, PrefabData& prefabData);
private:
	PrefabData m_data;
};

// Asset
class EngineAPI PrefabAsset : public Asset
{
public:
	using ResourceType = Prefab;

	using Asset::Asset;

	void serialize(nlohmann::json& j) const override;
	void deserialize(const nlohmann::json& j) override;
};
