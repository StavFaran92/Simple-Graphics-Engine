#include "memory/AssetInfo.h"

#include "core/Engine.h"
#include "memory/Assets.h"
#include "memory/AssetWrapper.h"
#include "fileSystem/EnginePath.h"

void AssetInfo::establishFilepath()
{
	if (!isTransient)
	{
		fullFilePath = Engine::get()->getProjectDirectory() + "/" + relativefilePath;
		std::filesystem::create_directories(std::filesystem::path(fullFilePath).parent_path());
	}
	else
	{
		fullFilePath = origFilePath;
	}
}

AssetInfo::AssetInfo(const AssetCreateDescriptor& assetDesc)
{
	importSettings = assetDesc.fillParams();

	name = assetDesc.name;
	aType = assetDesc.aType;
	origFilePath = assetDesc.origFilePath;
	assetDirectory = assetDesc.assetDirectory;
	attributes = assetDesc.attributes;
	isEngineOwned = assetDesc.isEngineOwned;
	isTransient = assetDesc.isTransient;
	isCompositeAsset = assetDesc.isCompositeAsset;
	targetDirectory = assetDesc.targetDirectory;

	// TODO this is a temporary fix to not break all the engine assets, it prevents me from using nested folder in the engine folder and should be fixed.
	if (isEngineOwned)
	{
		targetDirectory = EnginePath();
	}

	if (aType == AssetType::NONE)
	{
		logError("Asset type cannot be NONE.");
		return;
	}

	// Extract name
	if (!origFilePath.empty())
	{
		auto& path = std::filesystem::path(origFilePath);

		// Extract Name
		if (name.empty())
		{
			name = path.filename().stem().string();
		}
	}
	else if (isTransient)
	{
		logError("Cannot create a transient resource without original file path specified.");
		return;
	}

	// Generate UUID
	uuid = UUID::generate_uuid_v4();

	if (name.empty())
	{
		name = uuid;
	}

	// Extract Extension
	if (!origFilePath.empty())
	{
		auto& path = std::filesystem::path(origFilePath);

		if (path.has_extension())
		{
			ext = path.extension().string();
		}
	}
	if (ext.empty())
	{
		ext = getExtensionFromType(aType);

		if (ext.empty())
		{
			logError("Asset extension cannot be empty.");
			return;
		}
	}

	fileName = name + ext;

	relativefilePath = "";
	relativefilePath += targetDirectory.raw().generic_string();

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
}

void AssetInfo::update(const AssetUpdateDescriptor& uDesc)
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
		attributes[attrib.first] = attrib.second;
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

AssetWrapper<ResourceBase> AssetInfo::data() const
{
	return AssetWrapper<ResourceBase>(uuid);
}