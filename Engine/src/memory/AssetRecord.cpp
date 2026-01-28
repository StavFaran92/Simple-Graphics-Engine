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
		{"isValid", asset.isValid},
		{"importSettings", asset.importSettings},
		{"filename", asset.fileName},
		{"ext", asset.ext},
		{"createDescriptor", asset.createDescriptor},
	};
}

// Deserialization (from JSON)
void from_json(const nlohmann::json& j, AssetRecord& asset)
{
	j.at("uuid").get_to(asset.uuid); 
	j.at("relativefilePath").get_to(asset.relativefilePath);
	j.at("isValid").get_to(asset.isValid);
	j.at("importSettings").get_to(asset.importSettings);
	j.at("filename").get_to(asset.fileName);
	j.at("ext").get_to(asset.ext);
	j.at("createDescriptor").get_to(asset.createDescriptor);

	asset.establishFilepath();
}

void AssetRecord::establishFilepath()
{
	if (!createDescriptor.isTransient)
	{
		fullFilePath = Engine::get()->getProjectDirectory() + "/" + relativefilePath;
		std::filesystem::create_directories(std::filesystem::path(fullFilePath).parent_path());
	}
	else
	{
		fullFilePath = createDescriptor.sourcePath;
	}
}

AssetRecord::AssetRecord(AssetCreateDescriptor& assetDesc)
{
	createDescriptor = assetDesc;
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
	if (createDescriptor.isEngineOwned)
	{
		createDescriptor.targetDirectory = ScopedPath::EnginePath(createDescriptor.targetDirectory.relative());
	}

	if (createDescriptor.targetDirectory.type() == ScopedPath::Type::None)
	{
		createDescriptor.targetDirectory = ScopedPath::ContentPath("");
	}

	if (createDescriptor.aType == AssetType::NONE)
	{
		logError("Asset type cannot be NONE.");
		return;
	}

	// Extract name
	if (!createDescriptor.sourcePath.empty())
	{
		auto& path = std::filesystem::path(createDescriptor.sourcePath);

		// Extract Name
		if (createDescriptor.name.empty())
		{
			createDescriptor.name = path.filename().stem().string();
		}
	}
	else if (createDescriptor.isTransient)
	{
		logError("Cannot create a transient asset without original file path specified.");
		return;
	}

	// Generate UUID
	uuid = UUID::generate_uuid_v4();

	if (createDescriptor.name.empty())
	{
		createDescriptor.name = uuid;
	}

	// Extract Extension
	if (!createDescriptor.sourcePath.empty())
	{
		auto& path = std::filesystem::path(createDescriptor.sourcePath);

		if (path.has_extension())
		{
			ext = path.extension().string();
		}
	}
	if (ext.empty())
	{
		//ext = AssetFactory::getManager(createDescriptor.aType)->getRecommendedExtension(*this);
		ext = getExtensionFromType(createDescriptor.aType); // todo fix

		if (ext.empty())
		{
			logError("Asset extension cannot be empty.");
			return;
		}
	}

	fileName = createDescriptor.name + ext;

	relativefilePath = "";
	relativefilePath += createDescriptor.targetDirectory.scoped().generic_string();

	if (createDescriptor.isCompositeAsset)
	{
		createDescriptor.assetDirectory = createDescriptor.name;
	}

	if (!createDescriptor.assetDirectory.empty())
	{
		relativefilePath += "/" + createDescriptor.assetDirectory + "/";
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

void AssetRecord::update(const AssetUpdateDescriptor& uDesc)
{
	if (!uDesc.assetDirectory.empty())
	{
		createDescriptor.assetDirectory = uDesc.assetDirectory;
	}

	if (!uDesc.name.empty())
	{
		createDescriptor.name = uDesc.name;
	}

	for (const auto& attrib : uDesc.attributes)
	{
		createDescriptor.engineAttributes[attrib.first] = attrib.second;
	}

	fileName = createDescriptor.name + ext;

	relativefilePath = "";
	if (createDescriptor.isEngineOwned)
	{
		relativefilePath += "Engine/";
	}
	else
	{
		relativefilePath += "Content/";
	}

	if (!createDescriptor.assetDirectory.empty())
	{
		relativefilePath += createDescriptor.assetDirectory + "/";
	}

	relativefilePath += "/" + fileName;

	relativefilePath = std::filesystem::path(relativefilePath).lexically_normal().generic_string();

	establishFilepath();
}