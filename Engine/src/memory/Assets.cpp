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

std::vector<Asset> Assets::getAllAssetsOfType(AssetType aType) const
{
	if (aType == AssetType::NONE)
	{
		logError("Invalid asset type specified!");
		return {};
	}
	std::vector<Asset> result;
	for (const auto& [uuid, info] :m_assets)
	{
		if (info.aType == aType)
		{
			Asset asset = getAsset(uuid);
			result.push_back(asset);
		}
	}
	return result;
}

std::vector<Asset> Assets::getAllAssets() const
{
	std::vector<Asset> result;
	for (const auto& [uuid, info] : m_assets)
	{
		Asset asset = getAsset(uuid);
		result.push_back(asset);
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

void Assets::saveDirtyAssets()
{
	for (auto& [uuid, assetInfo] : m_assets)
	{
		if (assetInfo.isDirty)
		{
			Asset asset = getAsset(uuid);
			AssetFactory::getManager(assetInfo.aType)->save(asset, assetInfo);
			assetInfo.isDirty = false;
		}
	}
}

Asset Assets::getAsset(UUID uuid) const
{
	auto iter = m_assets.find(uuid);
	if (iter != m_assets.end())
	{
		return Asset(uuid);
	}
	logWarning("Could not locate asset: {}", uuid);
	return {};
}

const AssetInfo& Assets::getInfo(UUID uuid) const
{
	auto iter = m_assets.find(uuid);
	if (iter != m_assets.end())
	{
		return iter->second;
	}
	logWarning("Could not locate asset info: {}", uuid);
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
	Engine::get()->getContext()->getProjectAssetRegistry()->updateAssetRegistry(aInfo);
}

bool Assets::importAssetInner(AssetInfo& aInfo)
{
	std::string fileLocation = aInfo.origFilePath;

	// Validate input
	if (fileLocation.empty() || !std::filesystem::exists(fileLocation))
	{
		logError("Invalid asset path specified.");
		return false;
	}

	// Copy + Paste
	if (!AssetFactory::getManager(aInfo.aType)->copyFiles(fileLocation, aInfo))
	{
		logError("Failed to copy file from {} to resource folder", fileLocation);
		return false;
	}

	// Load
	ResourceWrapper<Resource> resource = AssetFactory::getManager(aInfo.aType)->load(aInfo);
	if (resource.isEmpty() || !resource.get())
	{
		logError("Failed to load file {}", fileLocation);
		return false;
	}


	// Add Asset
	aInfo.resource = resource;
	addAsset(aInfo);

	return true;
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

Asset Assets::getAssetFromPath(const std::string& path) const
{
	return Engine::get()->getMemoryManagementSystem()->getUUIDFromPath(path);
}

Asset Assets::getAssetFromName(const std::string& name) const
{
	return Engine::get()->getMemoryManagementSystem()->getUUIDFromName(name);
}

void Assets::updateAsset(Asset asset, const AssetUpdateDescriptor& uDesc)
{
	AssetInfo aInfo = getAsset(asset.getUID()).info();
	aInfo.update(uDesc);

	AssetFactory::getManager(aInfo.aType)->save(asset, aInfo); // todo check for non engine generated 
	updateRegistry(aInfo);

	m_assets[aInfo.uuid] = aInfo;

	logInfo("Successfully Updated asset: '" + aInfo.name + "'.");
}

void Assets::reimportAsset(UUID uuid)
{
	AssetInfo aInfo = getAsset(uuid).info();

	importAssetInner(aInfo);
}

void Assets::makeDirty(UUID uuid)
{
	AssetInfo aInfo = getAsset(uuid).info();
	aInfo.isDirty = true;
	m_assets[uuid] = aInfo;
}

std::vector<Asset> Assets::getAssetDependancies(UUID uuid) const
{
	std::vector<Asset> results;

	AssetInfo aInfo = getAsset(uuid).info();
	for (UUID depedantAssetUUID : aInfo.assetsDependancies)
	{
		results.push_back(getAsset(depedantAssetUUID));
	}
	return results;
}

//void Assets::addAssetDependency(UUID asset, UUID dependency)
//{
//	if (asset != dependency)
//	{
//		logError("Asset cannot depend on itself");
//		return;
//	}
//
//	if (!hasAsset(asset))
//	{
//		logError("Could not find asset {}", asset);
//		return;
//	}
//
//	if (!hasAsset(dependency))
//	{
//		logError("Could not find asset {}", dependency);
//		return;
//	}
//
//	AssetInfo aInfo = getAsset(asset);
//	aInfo.assetsDependancies.push_back(dependency);
//	m_assets[asset] = aInfo;
//}
//
//void Assets::removeAssetDependency(UUID asset, UUID dependency)
//{
//	if (asset != dependency)
//	{
//		logError("Asset cannot depend on itself");
//		return;
//	}
//
//	if (!hasAsset(asset))
//	{
//		logError("Could not find asset {}", asset);
//		return;
//	}
//
//	if (!hasAsset(dependency))
//	{
//		logError("Could not find asset {}", dependency);
//		return;
//	}
//
//	AssetInfo aInfo = getAsset(asset);
//	aInfo.assetsDependancies.erase(
//		std::remove(aInfo.assetsDependancies.begin(), aInfo.assetsDependancies.end(), dependency),
//		aInfo.assetsDependancies.end()
//	);
//	m_assets[asset] = aInfo;
//}

Asset Assets::importAsset(const std::string& fileLocation, AssetCreateDescriptor& desc)
{
	desc.origFilePath = fileLocation;
	AssetInfo aInfo(desc);

	if (!importAssetInner(aInfo))
	{
		return Asset::empty;
	}

	Asset asset(aInfo.uuid);

	return asset;
}

Asset Assets::createAsset(const ResourceWrapper<Resource>& resource, AssetCreateDescriptor& desc)
{
	// Add asset info
	AssetInfo aInfo(desc);
	aInfo.resource = resource;
	addAsset(aInfo);

	// Create asset
	Asset asset(aInfo.uuid);

	// Save asset
	AssetFactory::getManager(aInfo.aType)->save(asset, aInfo);

	return asset;
}

// TODO maybe remove this to a resource loader? doesnt really belong here...
ResourceWrapper<Resource> Assets::loadResource(const std::string& fileLocation, AssetCreateDescriptor& desc)
{
	desc.origFilePath = fileLocation;
	desc.isTransient = true;

	AssetInfo aInfo(desc);

	// Validate input
	if (fileLocation.empty() || !std::filesystem::exists(fileLocation))
	{
		logError("Invalid asset path specified.");
		return ResourceWrapper<Resource>::empty;
	}

	// Load
	ResourceWrapper<Resource> resource = AssetFactory::getManager(aInfo.aType)->load(aInfo);
	if (resource.isEmpty() || !resource.get())
	{
		logError("Failed to load file {}", fileLocation);
		return ResourceWrapper<Resource>::empty;
	}

	return resource;
}

void Assets::deleteAsset(Asset asset)
{
	AssetInfo aInfo = asset.info();
	Engine::get()->getMemoryManagementSystem()->removePathReference(aInfo.relativefilePath);
	Engine::get()->getContext()->getProjectAssetRegistry()->removeAssetRegistry(aInfo);
	m_assets.erase(aInfo.uuid);
}