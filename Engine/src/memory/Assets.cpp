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
#include "systems/UniqueNameManager.h"

#include <filesystem>

Assets::Assets()
{
	m_assets = {};
	Engine::get()->registerSubSystem<Assets>(this);
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
		assetInfo.resource = AssetFactory::getManager(assetInfo.aType)->load(assetInfo);
		m_assets[assetInfo.uuid] = assetInfo;
	}
}

const AssetInfo& Assets::getAsset(UUID uuid) const
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
	if (!aInfo.name.empty())
	{
		Engine::get()->getMemoryManagementSystem()->addNameReference(aInfo.name, aInfo.uuid);
	}
	Engine::get()->getMemoryManagementSystem()->addPathReference(aInfo.relativefilePath, aInfo.uuid); //TODO maybe use some naming convention here?
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
	return Engine::get()->getMemoryManagementSystem()->getUUIDFromPath(path);
}

UUID Assets::getAssetFromName(const std::string& name) const
{
	return Engine::get()->getMemoryManagementSystem()->getUUIDFromName(name);
}

void Assets::updateAsset(const AssetWrapper<ResourceBase>& asset, const AssetUpdateDescriptor& uDesc)
{
	AssetInfo aInfo = getAsset(asset.getUID());
	aInfo.update(uDesc);

	AssetFactory::getManager(aInfo.aType)->save(asset, aInfo); // todo check for non engine generated 
	updateRegistry(aInfo);

	m_assets[aInfo.uuid] = aInfo;

	logInfo("Successfully Updated asset: '" + aInfo.name + "'.");
}

AssetWrapper<ResourceBase> Assets::importAsset(const std::string& fileLocation, AssetCreateDescriptor& desc)
{
	desc.origFilePath = fileLocation;
	AssetInfo aInfo(desc);

	// Validate input
	if (fileLocation.empty() || !std::filesystem::exists(fileLocation))
	{
		logError("Invalid asset path specified.");
		return AssetWrapper<ResourceBase>::empty;
	}

	// Copy + Paste
	if (!AssetFactory::getManager(aInfo.aType)->copyFiles(fileLocation, aInfo))
	{
		logError("Failed to copy file from {} to resource folder", fileLocation);
		return AssetWrapper<ResourceBase>::empty;
	}

	// Load
	ResourceWrapper<ResourceBase> resource = AssetFactory::getManager(aInfo.aType)->load(aInfo);
	if (resource.isEmpty() || !resource.get())
	{
		logError("Failed to load file {}", fileLocation);
		return AssetWrapper<ResourceBase>::empty;
	}


	// Add Asset
	aInfo.resource = resource;
	addAsset(aInfo);

	AssetWrapper<ResourceBase> asset(aInfo.uuid);

	return asset;
}

AssetWrapper<ResourceBase> Assets::createAsset(const ResourceWrapper<ResourceBase>& resource, AssetCreateDescriptor& desc)
{
	// Add asset info
	AssetInfo aInfo(desc);
	aInfo.resource = resource;
	addAsset(aInfo);

	// Create asset
	AssetWrapper<ResourceBase> asset(aInfo.uuid);

	// Save asset
	AssetFactory::getManager(aInfo.aType)->save(asset, aInfo);

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
	Engine::get()->getMemoryManagementSystem()->removePathReference(aInfo.relativefilePath);
	Engine::get()->getContext()->getProjectAssetRegistry()->removeAssetRegistry(aInfo);
	m_assets.erase(aInfo.uuid);
}