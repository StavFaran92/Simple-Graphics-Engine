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
#include "memory/AssetManager.h"

#include <filesystem>

Assets::Assets()
{
	m_assets = {};
	Engine::get()->registerSubSystem<Assets>(this);
}

void Assets::addAsset(AssetRecord& aInfo)
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

	m_assets[aInfo.uuid] = aInfo;

	logInfo("Successfully Added asset: '" + aInfo.name + "'.");
}

AssetHandle<Asset> Assets::importAsset(AssetType aType, const std::string& fileLocation, AssetCreateDescriptor desc)
{
	// TODO validation

	AssetRecord aInfo(desc);
	aInfo.sourcePath = fileLocation;
	aInfo.aType = aType;
	aInfo.parse();

	// Validate input
	if (fileLocation.empty() || !std::filesystem::exists(fileLocation))
	{
		logError("Invalid asset path specified.");
		return false;
	}

	// Copy + Paste
	if (!AssetFactory::getManager(aType)->copyFiles(fileLocation, aInfo))
	{
		logError("Failed to copy file from {} to resource folder", fileLocation);
		return false;
	}

	// Create Asset
	Asset* asset = AssetFactory::getManager(aType)->createAsset(desc);
	if (!asset)
	{
		logError("Failed to Create Asset of type {}", aType);
		return false;
	}
	
	aInfo.asset = asset;

	addAsset(aInfo);

	return AssetHandle<Asset>(aInfo.uuid);
}

AssetHandle<Asset> Assets::promoteToAsset(ResourceWrapper<Resource> resource, AssetCreateDescriptor desc)
{
	// TODO validation

	AssetRecord aInfo(desc);
	aInfo.aType = desc.aType; // todo fix
	aInfo.parse();

	if (!AssetFactory::getManager(desc.aType)->saveToFile(resource, aInfo))
	{
		logError("Failed to save resource {} to file", resource.getUID());
		return false;
	}

	// Create Asset
	Asset* asset = AssetFactory::getManager(desc.aType)->createAsset(desc);
	if (!asset)
	{
		logError("Failed to Create Asset of type {}", desc.aType);
		return false;
	}

	aInfo.asset = asset;

	addAsset(aInfo);

	return AssetHandle<Asset>(aInfo.uuid);
}

std::vector<AssetHandle<Asset>> Assets::getAllAssetsOfType(AssetType aType) const
{
	if (aType == AssetType::NONE)
	{
		logError("Invalid asset type specified!");
		return {};
	}
	std::vector<AssetHandle<Asset>> result;
	for (const auto& [uuid, info] :m_assets)
	{
		if (info.aType == aType)
		{
			AssetHandle<Asset> asset = getAsset(uuid);
			result.push_back(asset);
		}
	}
	return result;
}

std::vector<const AssetRecord*> Assets::getAllRecordsOfType(AssetType aType) const
{
	std::vector<const AssetRecord*> records;

	const auto& handles = getAllAssetsOfType(aType);
	records.reserve(handles.size());

	for (const AssetHandle<Asset>& handle : handles)
	{
		records.push_back(&getInfo(handle.getUID()));
	}

	return records;
}

std::vector<AssetHandle<Asset>> Assets::getAllAssets() const
{
	std::vector<AssetHandle<Asset>> result;
	for (const auto& [uuid, info] : m_assets)
	{
		AssetHandle<Asset> asset = getAsset(uuid);
		result.push_back(asset);
	}
	return result;
}

std::vector<const AssetRecord*> Assets::getAllRecords() const
{
	std::vector<const AssetRecord*> records;
	records.reserve(m_assets.size());

	for (const auto& [id, record] : m_assets)
	{
		records.push_back(&record);
	}

	return records;
}

void Assets::loadAssetsDatabase()
{
	auto par = Engine::get()->getContext()->getProjectAssetRegistry();

	std::vector<AssetRecord> assets = par->getAllAssets();

	for (auto& assetInfo : assets)
	{
		m_assets[assetInfo.uuid] = assetInfo;
	}
}

void Assets::saveDirtyAssets()
{
	for (auto& [uuid, assetInfo] : m_assets)
	{
		if (assetInfo.isDirty())
		{
			AssetHandle<Asset> asset = getAsset(uuid);
			asset->save(asset.info());
			assetInfo.m_isDirty = false;
		}
	}
}

AssetHandle<Asset> Assets::getAsset(UUID uuid) const
{
	auto iter = m_assets.find(uuid);
	if (iter != m_assets.end())
	{
		return AssetHandle<Asset>(uuid);
	}
	logWarning("Could not locate asset: {}", uuid);
	return {};
}

const AssetRecord& Assets::getInfo(UUID uuid) const
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

void Assets::updateRegistry(const AssetRecord& aInfo)
{
	if (!aInfo.name.empty())
	{
		Engine::get()->getMemoryManagementSystem()->addNameReference(aInfo.name, aInfo.uuid);
	}
	Engine::get()->getMemoryManagementSystem()->addPathReference(aInfo.relativefilePath, aInfo.uuid); //TODO maybe use some naming convention here?
	Engine::get()->getContext()->getProjectAssetRegistry()->updateAssetRegistry(aInfo);
}

//bool Assets::importAssetInner(AssetRecord& aInfo)
//{
//	std::string fileLocation = aInfo.createDescriptor->origFilePath;
//
//	// Validate input
//	if (fileLocation.empty() || !std::filesystem::exists(fileLocation))
//	{
//		logError("Invalid asset path specified.");
//		return false;
//	}
//
//	// Copy + Paste
//	if (!AssetFactory::getManager(aInfo.createDescriptor->aType)->copyFiles(fileLocation, aInfo))
//	{
//		logError("Failed to copy file from {} to resource folder", fileLocation);
//		return false;
//	}
//
//	addAsset(aInfo);
//
//	return true;
//}

std::string Assets::getAlias(UUID uid) const
{
	auto iter = m_assets.find(uid);
	if (iter != m_assets.end())
	{
		return iter->second.name;
	}
	return "N/A";

}

AssetHandle<Asset> Assets::getAssetFromPath(const std::string& path) const
{
	return Engine::get()->getMemoryManagementSystem()->getUUIDFromPath(path);
}

AssetHandle<Asset> Assets::getAssetFromName(const std::string& name) const
{
	return Engine::get()->getMemoryManagementSystem()->getUUIDFromName(name);
}

//void Assets::updateAsset(AssetHandle<Asset> asset, const AssetUpdateDescriptor& uDesc)
//{
//	AssetRecord aInfo = getAsset(asset.getUID()).info();
//	aInfo.update(uDesc);
//
//	AssetFactory::getManager(aInfo.aType)->save(asset, aInfo); // todo check for non engine generated 
//	updateRegistry(aInfo);
//
//	m_assets[aInfo.uuid] = aInfo;
//
//	logInfo("Successfully Updated asset: '" + aInfo.name + "'.");
//}

//void Assets::reimportAsset(UUID uuid)
//{
//	AssetRecord aInfo = getAsset(uuid).info();
//
//	importAssetInner(aInfo);
//}

void Assets::makeDirty(UUID uuid)
{
	AssetRecord aInfo = getAsset(uuid).info();
	aInfo.makeDirty();
	m_assets[uuid] = aInfo;
}

//std::vector<AssetHandle<Asset>> Assets::getAssetDependancies(UUID uuid) const
//{
//	std::vector<Asset> results;
//
//	AssetRecord aInfo = getAsset(uuid).info();
//	for (UUID depedantAssetUUID : aInfo.assetsDependancies)
//	{
//		results.push_back(getAsset(depedantAssetUUID));
//	}
//	return results;
//}

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

//AssetHandle<Asset> Assets::importAsset(const std::string& fileLocation, AssetCreateDescriptor& desc)
//{
//	desc.origFilePath = fileLocation;
//	AssetRecord aInfo(desc);
//
//	if (!importAssetInner(aInfo))
//	{
//		return AssetHandle<Asset>::empty;
//	}
//
//	AssetHandle<Asset> asset(aInfo.uuid);
//
//	return asset;
//}

//AssetHandle<Asset> Assets::createAsset(const ResourceWrapper<Resource>& resource, AssetCreateDescriptor& desc)
//{
//	// Add asset info
//	AssetRecord aInfo(desc);
//	aInfo.resourceID = resource.getUID();
//	addAsset(aInfo);
//
//	// Create asset
//	AssetHandle<Asset> asset(aInfo.uuid);
//
//	// Save asset
//	AssetFactory::getManager(aInfo.aType)->save(asset, aInfo);
//
//	return asset;
//}

void Assets::updateAsset(const AssetHandle<Asset>& asset, const AssetUpdateDescriptor& uDesc)
{
	AssetRecord aInfo = asset.info();
	aInfo.update(uDesc);

	updateRegistry(aInfo);
	m_assets[aInfo.uuid] = aInfo;

	logInfo("Successfully Updated asset: '" + aInfo.name + "'.");
}

void Assets::deleteAsset(AssetHandle<Asset> asset)
{
	AssetRecord aInfo = asset.info();
	Engine::get()->getMemoryManagementSystem()->removePathReference(aInfo.relativefilePath);
	Engine::get()->getContext()->getProjectAssetRegistry()->removeAssetRegistry(aInfo);
	m_assets.erase(aInfo.uuid);
}