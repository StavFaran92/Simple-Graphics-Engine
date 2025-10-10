#pragma once

#include "memory/Asset.h"
#include "core/Core.h"
#include "runtime/Entity.h"
#include "serialize/Archiver.h"

struct PrefabImportSettings : public AssetCreateDescriptor
{};

struct PrefabAssetManager : public AssetManager
{
	bool copyFiles(const std::string& fileLocation, AssetInfo& aInfo) override;
	ResourceWrapper<ResourceBase> load(AssetInfo& aInfo) override;
	void save(const ResourceWrapper<ResourceBase>& mat, const AssetInfo& aInfo) override;
};

class EngineAPI Prefab : public ResourceBase
{
public:
	static ResourceWrapper<Prefab> import(const std::string& fileLocation, PrefabImportSettings desc);
	static ResourceWrapper<Prefab> create(const Entity& e);
	static void save(const ResourceWrapper<Prefab>& prefab, AssetInfo aInfo);

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