#pragma once

#include "memory/Asset.h"
#include "core/Core.h"
#include "runtime/Entity.h"
#include "serialize/Archiver.h"

struct PrefabImportSettings : public AssetCreateDescriptor
{};

// Asset IO Manager
struct PrefabAssetManager : public AssetManager
{
	bool copyFiles(const std::string& fileLocation, AssetRecord& aInfo) override;
	ResourceWrapper<Resource> load(AssetRecord& aInfo) override;
	void save(AssetHandle<Asset> asset, const AssetRecord& aInfo) override;
};

// Resource
class EngineAPI Prefab : public Resource
{
public:
	static ResourceWrapper<Prefab> create(const Entity& e);
	static void save(const ResourceWrapper<Prefab>& prefab, AssetRecord aInfo);

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
	static AssetHandle<PrefabAsset> import(const std::string& fileLocation, PrefabImportSettings aDesc = {});
};