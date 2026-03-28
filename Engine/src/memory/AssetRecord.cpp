#include "memory/AssetRecord.h"

#include "core/Engine.h"
#include "memory/Assets.h"
#include "memory/AssetHandle.h"
#include "fileSystem/ScopedPath.h"
#include "memory/AssetFactory.h"
#include "memory/Asset.h"
#include "core/Logger.h"

nlohmann::json serializeAsset(const Ref<Asset>& asset)
{
	nlohmann::json j;
	asset->serialize(j);
	return j;
}



// Serialization (to JSON)
void to_json(nlohmann::json& j, const AssetRecord& r)
{
	j = nlohmann::json{
		{"name", r.name},
		{"uuid", r.uuid},
		{"sourcePath", r.sourcePath},
		{"aType", r.aType},
		{"isEngineOwned", r.isEngineOwned},
		{"assetDirectory", r.assetDirectory},
		{"relativefilePath", r.relativefilePath},
		{"asset", serializeAsset(r.asset)}
	};
}

void from_json(const nlohmann::json& j, AssetRecord& r)
{
	j.at("name").get_to(r.name);
	j.at("uuid").get_to(r.uuid);
	j.at("sourcePath").get_to(r.sourcePath);
	j.at("aType").get_to(r.aType);
	j.at("isEngineOwned").get_to(r.isEngineOwned);
	j.at("assetDirectory").get_to(r.assetDirectory);
	j.at("relativefilePath").get_to(r.relativefilePath);

	nlohmann::json jsonAsset = j.at("asset");
	Ref<Asset> asset = AssetFactory::getManager(r.aType)->deserializeAsset(jsonAsset);
	r.asset = asset;

	if (r.isEngineOwned)
	{
		r.targetDirectory = ScopedPath::EnginePath(r.relativefilePath);
	}
	else
	{
		r.targetDirectory = ScopedPath::ContentPath(r.relativefilePath);
	}
}

AssetRecord::AssetRecord(const AssetBuildDescriptor& assetDesc)
{
	name = assetDesc.name;
	aType = assetDesc.aType;
	isEngineOwned = assetDesc.isEngineOwned;
	targetDirectory = assetDesc.targetDirectory;

	uuid = UUID::generate_uuid_v4();
}

void AssetRecord::parse()
{
	m_isParsed = true;
}

bool AssetRecord::isParsed() const
{
	return m_isParsed;
}

void AssetRecord::makeDirty()
{
	m_isSerializationDirty = true;
	m_isResourceDirty = true;
}

void AssetRecord::sync()
{
	m_isResourceDirty = false;
}

bool AssetRecord::isSerializationDirty() const
{
	return m_isSerializationDirty;
}

bool AssetRecord::isResourceDirty() const
{
	return m_isResourceDirty;
}
