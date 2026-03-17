#include "memory/Assets.h"

#include "core/CacheSystem.h"
#include "memory/AssetFactory.h"
#include "memory/RegisterManagers.h"
#include "runtime/Context.h"

#include <filesystem>

Assets::Assets()
{
	m_assets = {};
	Engine::get()->registerSubSystem<Assets>(this);

	registerAllManagers();
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

void Assets::updateAssetInner(AssetRecord& aInfo)
{
	updateRegistry(aInfo);
	m_assets[aInfo.uuid] = aInfo;

	logInfo("Successfully Updated asset: '" + aInfo.name + "'.");
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
		//Ref<Asset> asset = AssetFactory::getManager(assetInfo.aType)->deserializeAsset();
		//assetInfo.asset = asset;

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
			//asset->save(asset.info()); // TODO fix
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

void Assets::deleteAsset(UUID uuid)
{
	auto asset = getAsset(uuid);
	AssetRecord aInfo = asset.info();
	Engine::get()->getMemoryManagementSystem()->removePathReference(aInfo.relativefilePath);
	Engine::get()->getContext()->getProjectAssetRegistry()->removeAssetRegistry(aInfo);
	m_assets.erase(aInfo.uuid);
}

ScopedPath calculateAssetDestinationPath(
	bool isEngineOwned,
	const std::filesystem::path& relativeFolder,
	const std::string& assetName,
	AssetType type)
{
	ScopedPath p = isEngineOwned ? ScopedPath::EnginePath() : ScopedPath::ContentPath();

	assert(type != AssetType::NONE);
	assert(!assetName.empty());

	std::string ext = getExtensionFromType(type);

	std::filesystem::path filename = assetName + ext;

	p.setPath(relativeFolder / filename);

	return p;
}

//ScopedPath calculateAssetDestinationPathCreate(const AssetBuildDescriptor& desc)
//{
//	// Determine root
//	ScopedPath p = desc.isEngineOwned ? ScopedPath::EnginePath() : ScopedPath::ContentPath();
//
//	// determine relative folder
//	std::filesystem::path relativefolder;
//	if (!desc.assetDirectory.empty())
//	{
//		relativefolder = desc.assetDirectory;
//	}
//
//	assert(desc.aType != AssetType::NONE);
//	assert(!desc.name.empty());
//
//	// determine file name
//	auto& path = std::filesystem::path(desc.name);
//	std::string name = path.filename().string();
//
//	// determine externsion
//	std::string ext = getExtensionFromType(desc.aType);
//
//	std::filesystem::path filename = name + ext;
//
//	p.setPath(relativefolder / filename);
//
//	return p;
//}

AssetHandle<Asset> Assets::createAsset(AssetBuildDescriptor& desc, ResourceBuildDescriptor& resourceDesc)
{
	AssetType type = desc.aType;

	// Obtain resource manager
	ResourceTypeManager* manager = AssetFactory::getManager(type);
	if (!manager)
	{
		logError("No ResourceTypeManager registered for asset type {}", static_cast<int>(type));
		return AssetHandle<Asset>::empty;
	}

	// Parse the resource descriptor
	manager->parse(resourceDesc);

	// Save the asset to disk
	ScopedPath dest = calculateAssetDestinationPath(desc.isEngineOwned, desc.assetDirectory, desc.name, desc.aType);

	// Verify target dir exists
	fs::create_directories(dest.absolute().parent_path());

	if(!manager->saveResource(resourceDesc, dest))
	{
		logError("Failed to save asset type {} to: {}", static_cast<int>(type), dest.absolute().string());
		return AssetHandle<Asset>::empty;
	}

	Ref<Asset> asset = manager->createAsset(desc, resourceDesc);
	if (!asset)
	{
		logError("Failed to create asset of type {}", static_cast<int>(type));
		return AssetHandle<Asset>::empty;
	}

	AssetRecord record(desc);
	record.parse();
	record.relativefilePath = dest.relative().string();
	record.asset = asset;
	addAsset(record);

	return AssetHandle<Asset>(record.uuid);
}

AssetHandle<Asset> Assets::importAsset(AssetBuildDescriptor& desc, ResourceLoadDescriptor& resourceDesc)
{
	AssetType type = desc.aType;

	ResourceTypeManager* manager = AssetFactory::getManager(type);
	if (!manager)
	{
		logError("No ResourceTypeManager registered for asset type {}", static_cast<int>(type));
		return AssetHandle<Asset>::empty;
	}

	// Parse the resource descriptor
	manager->parse(resourceDesc);

	ImportNode importNode;
	importNode.assetDesc = desc;

	if (!manager->importAsset(resourceDesc.sourcePath, importNode))
	{
		logError("Failed to import asset type {}", static_cast<int>(type));
		return AssetHandle<Asset>::empty;
	}

	// If asset is composed of multiple assets place them all in a dedicated directory
	if (importNode.dependencies.size() > 0)
	{
		desc.assetDirectory = desc.name;
	}

	AssetHandle<Asset> handle = createAssetAndChildrenFromNodeRecursive(importNode, desc);

	if (handle.isEmpty())
	{
		logWarning("Failed to create asset of type {}", static_cast<int>(type));
	}

	Engine::get()->getContext()->getProjectAssetRegistry()->save();

	return handle;
}

void Assets::updateAsset(UUID uuid, AssetUpdateDescriptor& desc, ResourceBuildDescriptor& resourceDesc)
{
	auto& record = getInfo(uuid);

	ResourceTypeManager* manager = AssetFactory::getManager(record.aType);

	if (!manager)
	{
		logError("No ResourceTypeManager registered for asset type {}", static_cast<int>(record.aType));
		return;
	}
	
	ScopedPath p = record.isEngineOwned ? ScopedPath::EnginePath() : ScopedPath::ContentPath();
	p.setPath(record.relativefilePath);

	manager->saveResource(resourceDesc, p);
}

AssetHandle<Asset> Assets::createAssetsFromImportNode(const ImportNode& node, const AssetBuildDescriptor& rootDesc)
{
	ResourceTypeManager* manager =
		AssetFactory::getManager(node.assetDesc.aType);

	if (!manager)
	{
		logError("No ResourceTypeManager registered for asset type {}",
			static_cast<int>(node.assetDesc.aType));
		return AssetHandle<Asset>::empty;
	}

	AssetHandle<Asset> created;

	AssetBuildDescriptor nodeDesc = node.assetDesc;
	nodeDesc.assetDirectory = rootDesc.assetDirectory;
	nodeDesc.isEngineOwned = rootDesc.isEngineOwned;
	nodeDesc.targetDirectory = rootDesc.targetDirectory;

	if (node.creationType == CreationType::Create)
	{
		if (!node.createDesc)
		{
			logError("ImportNode '{}' is missing createDesc", node.name);
			return AssetHandle<Asset>::empty;
		}
		created = createAsset(nodeDesc, *node.createDesc);
	}
	else // CreationType::Import
	{
		if (!node.loadDesc)
		{
			logError("ImportNode '{}' is missing loadDesc", node.name);
			return AssetHandle<Asset>::empty;
		}
		created = importAsset(nodeDesc, *node.loadDesc);
	}

	if (created.isEmpty())
	{
		logWarning("Failed to create asset '{}' from import node", node.name);
		return AssetHandle<Asset>::empty;
	}

	return created;
}

AssetHandle<Asset> Assets::createAssetAndChildrenFromNodeRecursive(const ImportNode& node, const AssetBuildDescriptor& rootDesc)
{
	// 1) Create this node
	AssetHandle<Asset> created = createAssetsFromImportNode(node, rootDesc);

	if (created.isEmpty())
	{
		logWarning("Failed to create asset '{}' from import node", node.name);
		return AssetHandle<Asset>::empty;
	}

	// 2) Recursively create and bind dependencies
	for (const auto& [slotName, childNode] : node.dependencies)
	{
		AssetHandle<Asset> child = createAssetAndChildrenFromNodeRecursive(childNode, rootDesc);

		if (child.isEmpty())
		{
			logWarning("Failed to create dependency '{}' for asset '{}'",
				childNode.name, node.name);
			continue;
		}

		// 3) Bind dependency to this asset
		created->bindDependency(slotName, child.getUID());
	}

	Engine::get()->getContext()->getProjectAssetRegistry()->updateAssetRegistry(created.info());
	

	return created;
}

void Assets::bindResourceToAsset(UUID uuid, ResourceID resID)
{
	AssetRecord newAssetInfo = getInfo(uuid);
	newAssetInfo.resourceID = resID;
	updateAssetInner(newAssetInfo);
}