#include "memory/Assets.h"

#include <GL/glew.h>


#include "texture/Texture.h"
#include "animation/Animation.h"
#include "animation/AnimationLoader.h"
#include "core/CacheSystem.h"
#include "geometry/ModelImporter.h"
#include "core/Factory.h"
#include "render/ShaderBuilder.h"
#include "memory/AssetFactory.h"

#include <filesystem>

Assets::Assets()
{
	m_assets = {};
	Engine::get()->registerSubSystem<Assets>(this);
}

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

	if (aType == AssetType::NONE)
	{
		logError("Asset type cannot be NONE.");
		return;
	}

	if (!origFilePath.empty())
	{
		auto& path = std::filesystem::path(origFilePath);

		// Extract Name
		if (name.empty())
		{
			name = path.filename().stem().string();
		}
		ext = path.extension().string();
		fileName = path.filename().string();
	}
	else if(isTransient)
	{
		logError("Cannot create a transient resource without original file path specified.");
		return;
	}

	if (!assetDesc.customUUID.empty())
	{
		uuid = assetDesc.customUUID;
	}
	else
	{
		uuid = UUID::generate_uuid_v4();
	}

	if (name.empty())
	{
		name = uuid;
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

	if (fileName.empty())
	{
		fileName = name + ext;
	}

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

void Assets::addAsset(AssetInfo& aInfo)
{
	if (aInfo.aType == AssetType::NONE)
	{
		logError("Invalid asset type specified!");
		return;
	}
	if (aInfo.relativefilePath.empty())
	{
		logError("Non transient asset must have a file path specified.");
		return;
	}
	if (aInfo.uuid.empty())
	{
		logError("Asset must have a UUID");
		return;
	}

	updateRegistry(aInfo);

	aInfo.isValid = true;
	m_assets[aInfo.uuid] = aInfo;

	logInfo("Successfully Added asset: '" + aInfo.name + "'.");
}

std::vector<AssetInfo> Assets::getAllAssetsOfType(AssetType aType) const
{
	if (aType == AssetType::NONE)
	{
		logError("Invalid asset type specified!");
		return {};
	}
	std::vector<AssetInfo> result;
	for (const auto& asset : m_assets)
	{
		if (asset.second.aType == aType)
		{
			result.push_back(asset.second);
		}
	}
	return result;
}

std::vector<AssetInfo> Assets::getAllAssets() const
{
	std::vector<AssetInfo> result;
	for (const auto& asset : m_assets)
	{
		result.push_back(asset.second);
	}
	return result;
}

void Assets::loadAssetsDatabase()
{
	auto par = Engine::get()->getContext()->getProjectAssetRegistry();

	std::vector<AssetInfo> assets = par->getAllAssets();

	for (auto& assetInfo : assets)
	{
		assetInfo.data = AssetFactory::getManager(assetInfo.aType)->load(assetInfo);
		m_assets[assetInfo.uuid] = assetInfo;
	}
}

AssetInfo Assets::getAsset(UUID uuid) const
{
	auto iter = m_assets.find(uuid);
	if (iter != m_assets.end())
	{
		return iter->second;
	}
	return {};
}

bool Assets::hasAsset(UUID uuid) const
{
	auto iter = m_assets.find(uuid);
	if (iter != m_assets.end())
	{
		return true;
	}
	return false;
}

void Assets::updateRegistry(const AssetInfo& aInfo)
{
	Engine::get()->getMemoryManagementSystem()->addAssociation(aInfo.relativefilePath, aInfo.uuid); //TODO maybe use some naming convention here?
	Engine::get()->getContext()->getProjectAssetRegistry()->addAssetRegistry(aInfo);
}

std::string Assets::getAlias(UUID uid) const
{
	auto iter = m_assets.find(uid);
	if (iter != m_assets.end())
	{
		return iter->second.name;
	}
	return "N/A";

}

UUID Assets::getAssetFromPath(const std::string& path) const
{
	return Engine::get()->getMemoryManagementSystem()->getAssociation(path);
}

void Assets::updateAsset(const ResourceWrapper<ResourceBase>& asset, const AssetUpdateDescriptor& uDesc)
{
	AssetInfo aInfo = getAsset(asset.getUID());
	aInfo.update(uDesc);

	AssetFactory::getManager(aInfo.aType)->save(asset, aInfo); // todo check for non engine generated 
	updateRegistry(aInfo);

	m_assets[aInfo.uuid] = aInfo;
	asset.get()->m_assetInfo = aInfo;

	logInfo("Successfully Updated asset: '" + aInfo.name + "'.");
}

ResourceWrapper<ResourceBase> Assets::importAsset(const std::string& fileLocation, AssetCreateDescriptor& desc)
{
	desc.origFilePath = fileLocation;
	AssetInfo aInfo(desc);

	// Validate input
	if (fileLocation.empty() || !std::filesystem::exists(fileLocation))
	{
		logError("Invalid asset path specified.");
		return ResourceWrapper<ResourceBase>::empty;
	}

	// Copy + Paste
	if (!AssetFactory::getManager(aInfo.aType)->copyFiles(fileLocation, aInfo))
	{
		logError("Failed to copy file from {} to resource folder", fileLocation);
		return ResourceWrapper<ResourceBase>::empty;
	}

	// Load
	ResourceWrapper<ResourceBase> asset = AssetFactory::getManager(aInfo.aType)->load(aInfo);
	if (asset.isEmpty() || !asset.get())
	{
		logError("Failed to load file {}", fileLocation);
		return ResourceWrapper<ResourceBase>::empty;
	}

	aInfo.data = asset;

	// Add Asset
	addAsset(aInfo);
	asset.get()->m_assetInfo = aInfo;

	return asset;
}

ResourceWrapper<ResourceBase> Assets::createAsset(const ResourceWrapper<ResourceBase>& asset, AssetCreateDescriptor& desc)
{
	desc.customUUID = asset.getUID();

	AssetInfo aInfo(desc);

	AssetFactory::getManager(aInfo.aType)->save(asset, aInfo);

	asset.get()->m_assetInfo = aInfo;
	aInfo.data = asset;
	addAsset(aInfo);

	return asset;
}

// TODO maybe remove this to a resource loader? doesnt really belong here...
ResourceWrapper<ResourceBase> Assets::loadResource(const std::string& fileLocation, AssetCreateDescriptor& desc)
{
	desc.origFilePath = fileLocation;
	desc.isTransient = true;

	AssetInfo aInfo(desc);

	// Validate input
	if (fileLocation.empty() || !std::filesystem::exists(fileLocation))
	{
		logError("Invalid asset path specified.");
		return ResourceWrapper<ResourceBase>::empty;
	}

	// Load
	ResourceWrapper<ResourceBase> resource = AssetFactory::getManager(aInfo.aType)->load(aInfo);
	if (resource.isEmpty() || !resource.get())
	{
		logError("Failed to load file {}", fileLocation);
		return ResourceWrapper<ResourceBase>::empty;
	}

	return resource;
}

void Assets::deleteAsset(const AssetInfo& aInfo)
{
	Engine::get()->getMemoryManagementSystem()->removeAssociation(aInfo.relativefilePath);
	Engine::get()->getContext()->getProjectAssetRegistry()->removeAssetRegistry(aInfo);
	m_assets.erase(aInfo.uuid);
}