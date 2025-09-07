#pragma once

#include "memory/Asset.h"
#include "core/Core.h"
#include "runtime/Entity.h"
#include "serialize/Archiver.h"

struct PrefabImportSettings : BaseAssetParameters
{};

class EngineAPI Prefab : public Asset
{
public:
	static Resource<Prefab> import(const std::string& fileLocation, const PrefabImportSettings& settings);
	static Resource<Prefab> loadTransient(const std::string& fileLocation, const PrefabImportSettings& settings);
	static Resource<Prefab> create(const Entity& e, AssetInfo& aInfo);

	void Instansiate();

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(m_serializedPrefab);
	}

private:
	static void extractChildrenRecursive(const Entity& e, Resource<Prefab>& prefab);
private:
	std::vector<SerializedEntity> m_serializedPrefab;
};