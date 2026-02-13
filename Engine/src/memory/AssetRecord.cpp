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

	asset.establishFilepath();
}

void AssetRecord::establishFilepath()
{
	if (!isTransient)
	{
		fullFilePath = Engine::get()->getProjectDirectory() + "/" + relativefilePath;
		std::filesystem::create_directories(std::filesystem::path(fullFilePath).parent_path());
	}
	else
	{
		fullFilePath = sourcePath;
	}
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
}

void AssetRecord::parse()
{
	//importSettings = createDescriptor.fillParams();

	//name = createDescriptor.name;
	//aType = createDescriptor.aType;
	//origFilePath = createDescriptor.origFilePath;
	//assetDirectory = createDescriptor.assetDirectory;
	//attributes = createDescriptor.attributes;
	//isEngineOwned = createDescriptor.isEngineOwned;
	//isTransient = createDescriptor.isTransient;
	//isCompositeAsset = createDescriptor.isCompositeAsset;
	//targetDirectory = createDescriptor.targetDirectory;

	// TODO this is a temporary fix to not break all the engine assets, it prevents me from using nested folder in the engine folder and should be fixed.
	if (isEngineOwned)
	{
		targetDirectory = ScopedPath::EnginePath(targetDirectory.relative());
	}

	if (targetDirectory.type() == ScopedPath::Type::None)
	{
		targetDirectory = ScopedPath::ContentPath("");
	}

	if (aType == AssetType::NONE)
	{
		logError("Asset type cannot be NONE.");
		return;
	}

	// Extract name
	if (!sourcePath.empty())
	{
		auto& path = std::filesystem::path(sourcePath);

		// Extract Name
		if (name.empty())
		{
			name = path.filename().stem().string();
		}
	}
	else if (isTransient)
	{
		logError("Cannot create a transient asset without original file path specified.");
		return;
	}

	// Generate UUID
	uuid = UUID::generate_uuid_v4();

	if (name.empty())
	{
		name = uuid;
	}

	// Extract Extension
	if (!sourcePath.empty())
	{
		auto& path = std::filesystem::path(sourcePath);

		if (path.has_extension())
		{
			ext = path.extension().string();
		}
	}
	if (ext.empty())
	{
		//ext = AssetFactory::getManager(createDescriptor.aType)->getRecommendedExtension(*this);
		ext = getExtensionFromType(aType); // todo fix

		if (ext.empty())
		{
			logError("Asset extension cannot be empty.");
			return;
		}
	}

	fileName = name + ext;

	relativefilePath = "";
	relativefilePath += targetDirectory.scoped().generic_string();

	if (isCompositeAsset)
	{
		assetDirectory = name;
	}

	if (!assetDirectory.empty())
	{
		relativefilePath += "/" + assetDirectory + "/";
	}

	relativefilePath += "/" + fileName;

	relativefilePath = std::filesystem::path(relativefilePath).lexically_normal().generic_string();

	establishFilepath();

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

	establishFilepath();
}