#include "memory/AssetRecord.h"

#include "memory/Assets.h"
#include "fileSystem/ScopedPath.h"
#include "memory/AssetFactory.h"
#include "memory/Asset.h"
#include "core/Logger.h"

nlohmann::json serializeAsset(const Ref<Asset>& asset)
{
	return nlohmann::json::parse(asset->metadataToJson());
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
		{"ext", r.ext},
		{"visibility", r.visibility},
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
	j.at("visibility").get_to(r.visibility);
	j.at("ext").get_to(r.ext);
	j.at("assetDirectory").get_to(r.assetDirectory);

	Ref<Asset> asset = AssetFactory::getManager(r.aType)->makeAsset();
	try
	{
		asset->metadataFromJson(j.at("asset").dump());
	}
	catch (const std::exception& e)
	{
		logError("Failed to load asset '{}' ({}): {}", r.name, r.uuid.str(), e.what());
	}
	asset->uuid = r.uuid;
	r.asset = asset;
}

AssetRecord::AssetRecord(const AssetBuildDescriptor& assetDesc)
{
	name = assetDesc.name;
	aType = assetDesc.aType;
	isEngineOwned = assetDesc.isEngineOwned;
	assetDirectory = (assetDesc.targetDirectory.relative() / assetDesc.assetDirectory).string();
	visibility = assetDesc.visibility;

	uuid = assetDesc.hintUUID == EMPTY_UUID ? UUID::generate_uuid_v4() : assetDesc.hintUUID;
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

bool AssetRecord::isVisible() const
{
	return !(isEngineOwned && visibility == AssetVisibility::Internal);
}

std::string AssetRecord::getAbsolutePath() const
{
	return getScopedPath().absolute().string();
}

ScopedPath AssetRecord::getScopedPath() const
{
	ScopedPath p = isEngineOwned ? ScopedPath::EnginePath() : ScopedPath::ContentPath();
	p.setPath(std::filesystem::path(assetDirectory) / getFilename());
	return p;
}

std::string AssetRecord::getFilename() const
{
	return name + ext;
}
