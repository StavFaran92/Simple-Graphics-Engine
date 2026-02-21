#include "memory/AssetRecord.h"

#include "core/Engine.h"
#include "memory/Assets.h"
#include "memory/AssetHandle.h"
#include "fileSystem/ScopedPath.h"
#include "memory/AssetFactory.h"
#include "memory/Asset.h"
#include "core/Logger.h"

// Serialization (to JSON)
void to_json(nlohmann::json& j, const AssetRecord& asset)
{
	j = nlohmann::json{
		{"uuid", asset.uuid},
		{"relativefilePath", asset.relativefilePath},
		{"importSettings", asset.importSettings},
		{"filename", asset.fileName},
		{"ext", asset.ext}
	};
}

// Deserialization (from JSON)
void from_json(const nlohmann::json& j, AssetRecord& asset)
{
	j.at("uuid").get_to(asset.uuid); 
	j.at("relativefilePath").get_to(asset.relativefilePath);
	j.at("importSettings").get_to(asset.importSettings);
	j.at("filename").get_to(asset.fileName);
	j.at("ext").get_to(asset.ext);

	//asset.establishFilepath();
}

AssetRecord::AssetRecord(AssetCreateDescriptor& assetDesc)
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