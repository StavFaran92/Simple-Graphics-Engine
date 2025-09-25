#pragma once

#include "memory/Asset.h"
#include "core/Core.h"
#include "runtime/Entity.h"
#include "serialize/Archiver.h"

struct PrefabImportSettings : BaseAssetParameters
{};

class EngineAPI Prefab : public ResourceBase
{
public:
	static ResourceWrapper<Prefab> import(const std::string& fileLocation, const PrefabImportSettings& settings);
	static ResourceWrapper<Prefab> loadTransient(const std::string& fileLocation, const PrefabImportSettings& settings);
	static ResourceWrapper<Prefab> create(const Entity& e, const AssetDescriptor& aDesc);

	void Instansiate();

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(m_serializedPrefab);
	}

private:
	static void extractChildrenRecursive(const Entity& e, ResourceWrapper<Prefab>& prefab);
private:
	std::vector<SerializedEntity> m_serializedPrefab;
};