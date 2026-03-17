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
		{"engineAttributes", r.engineAttributes},
		{"isEngineOwned", r.isEngineOwned},
		{"assetDirectory", r.assetDirectory},
		{"fileName", r.fileName},
		{"relativefilePath", r.relativefilePath},
		{"ext", r.ext},
		{"asset", serializeAsset(r.asset)}
	};
}

void from_json(const nlohmann::json& j, AssetRecord& r)
{
	j.at("name").get_to(r.name);
	j.at("uuid").get_to(r.uuid);
	j.at("sourcePath").get_to(r.sourcePath);
	j.at("aType").get_to(r.aType);
	j.at("engineAttributes").get_to(r.engineAttributes);
	j.at("isEngineOwned").get_to(r.isEngineOwned);
	j.at("assetDirectory").get_to(r.assetDirectory);
	j.at("fileName").get_to(r.fileName);
	j.at("relativefilePath").get_to(r.relativefilePath);
	j.at("ext").get_to(r.ext);

	nlohmann::json jsonAsset = j.at("asset");
	Ref<Asset> asset = AssetFactory::getManager(r.aType)->deserializeAsset(jsonAsset);
	r.asset = asset;
}

AssetRecord::AssetRecord(const AssetCreateDescriptor& assetDesc)
{
	name = assetDesc.name;
	aType = assetDesc.aType;
	sourcePath = assetDesc.sourcePath;
	engineAttributes = assetDesc.engineAttributes;
	isEngineOwned = assetDesc.isEngineOwned;
	isTransient = assetDesc.isTransient;
	isCompositeAsset = assetDesc.isCompositeAsset;
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
	m_isDirty = true;
}

bool AssetRecord::isDirty() const
{
	return m_isDirty;
}

void AssetRecord::update(const AssetUpdateDescriptor& uDesc)
{
	if (!uDesc.assetDirectory.empty())
	{
		assetDirectory = uDesc.assetDirectory;
	}

	if (!uDesc.name.empty())
	{
		name = uDesc.name;
	}

	for (const auto& attrib : uDesc.attributes)
	{
		engineAttributes[attrib.first] = attrib.second;
	}

	fileName = name + ext;

	relativefilePath = "";
	if (isEngineOwned)
	{
		relativefilePath += "Engine/";
	}
	else
	{
		relativefilePath += "Content/";
	}

	if (!assetDirectory.empty())
	{
		relativefilePath += assetDirectory + "/";
	}

	relativefilePath += "/" + fileName;

	relativefilePath = std::filesystem::path(relativefilePath).lexically_normal().generic_string();

	//establishFilepath();
}