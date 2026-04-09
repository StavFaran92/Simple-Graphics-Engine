#include "memory/Assets.h"

#include "core/CacheSystem.h"
#include "memory/AssetFactory.h"
#include "memory/RegisterManagers.h"
#include "runtime/Context.h"
#include "runtime/Scene.h"
#include "utils/RandomNameGenerator.h"
#include "systems/UniqueNameManager.h"

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

	// Post initialize
	for (auto& [uuid, record] : m_assets)
	{
		// If its a scene we add it to the scene manager
		if (record.aType == AssetType::SCENE)
		{
			AssetHandle<SceneAsset> sceneAsset(uuid);
			Engine::get()->getContext()->addScene(sceneAsset);
		}
	}

	// TODO store active scene in some settings file and load it
}

void Assets::saveDirtyAssets()
{
	for (auto& [uuid, assetInfo] : m_assets)
	{
		if (assetInfo.isSerializationDirty())
		{
			// There is a clear design flaw here,
			// I have to save asset file AND asset meta data
			// What I should be doing is store everything: 
			// metadata and asset data in the asset file and remove the projectassetregistry json entirely.

			// Save asset file
			AssetHandle<Asset> asset = getAsset(uuid);
			std::unique_ptr<ResourceBuildDescriptor> buildDesc = AssetFactory::getManager(assetInfo.aType)->makeResourceBuildDescriptor();
			asset->fillBuildDescriptor(*buildDesc);

			ResourceTypeManager* manager = AssetFactory::getManager(assetInfo.aType);
			if (!manager)
			{
				logError("No ResourceTypeManager registered for asset type {}", static_cast<int>(assetInfo.aType));
				continue;
			}

			ScopedPath dest = assetInfo.getScopedPath();

			if (!manager->saveResource(*buildDesc, dest))
			{
				logError("Failed to save asset type {} to: {}", static_cast<int>(assetInfo.aType), dest.absolute().string());
				continue;
			}

			// save asset metadata
			Engine::get()->getContext()->getProjectAssetRegistry()->updateAssetRegistry(assetInfo);

			//asset->save(asset.info()); // TODO fix
			assetInfo.m_isSerializationDirty = false;
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

	Engine::get()->getMemoryManagementSystem()->addPathReference(aInfo.getScopedPath().scoped().string(), aInfo.uuid); //TODO maybe use some naming convention here?
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

void Assets::sync(UUID uuid)
{
	AssetRecord aInfo = getAsset(uuid).info();
	aInfo.sync();
	m_assets[uuid] = aInfo;
}

void Assets::deleteAsset(UUID uuid)
{
	auto asset = getAsset(uuid);
	AssetRecord aInfo = asset.info();
	Engine::get()->getMemoryManagementSystem()->removePathReference(aInfo.getScopedPath().scoped().string());
	Engine::get()->getMemoryManagementSystem()->removeNameReference(aInfo.name);
	Engine::get()->getContext()->getProjectAssetRegistry()->removeAssetRegistry(aInfo);
	std::filesystem::remove(aInfo.getAbsolutePath());
	m_assets.erase(aInfo.uuid);
}

ScopedPath calculateAssetDestinationPath(
	bool isEngineOwned,
	const std::filesystem::path& targetFolder,
	const std::filesystem::path& parentFolder,
	const std::string& assetName,
	AssetType type)
{
	ScopedPath p = isEngineOwned ? ScopedPath::EnginePath() : ScopedPath::ContentPath();

	assert(type != AssetType::NONE);
	assert(!assetName.empty());

	std::string ext = getExtensionFromType(type);

	std::filesystem::path filename = assetName + ext;

	p.setPath(targetFolder / parentFolder / filename);

	return p;
}

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

	if (desc.name.empty())
	{
		desc.name = RandomNameGenerator::generateName();
		logInfo("Empty name specified for asset, generating random name {}", desc.name);
	}

	// TODO i should check for duplicate names here
	//if (Engine::get()->getSubSystem<UniqueNameManager>()->isNameExists(desc.name, desc.targetDirectory))
	//{
	//	logError("Asset ");
	//	return AssetHandle<Asset>::empty;
	//}

	// Parse the resource descriptor
	manager->parse(resourceDesc);

	// Save the asset to disk
	ScopedPath dest = calculateAssetDestinationPath(desc.isEngineOwned, desc.targetDirectory.relative(), desc.assetDirectory, desc.name, desc.aType);

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
	record.asset = asset;
	record.ext = getExtensionFromType(type);
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

void Assets::updateAsset(UUID uuid, AssetUpdateDescriptor& desc, ResourceBuildDescriptor* resourceDesc)
{
	auto& record = getInfo(uuid);

	ResourceTypeManager* manager = AssetFactory::getManager(record.aType);

	if (!manager)
	{
		logError("No ResourceTypeManager registered for asset type {}", static_cast<int>(record.aType));
		return;
	}

	std::string newName = record.name;
	if (!desc.name.empty())
	{
		newName = desc.name;
	}

	ScopedPath dest = calculateAssetDestinationPath(record.isEngineOwned, desc.targetDirectory.relative(), record.assetDirectory, newName, record.aType);

	// Verify target dir exists
	fs::create_directories(dest.absolute().parent_path());

	// Resource has changed so we must save it 
	if (resourceDesc && !manager->saveResource(*resourceDesc, dest))
	{
		logError("Failed to save resource type {} to: {}", static_cast<int>(record.aType), dest.absolute().string());
		return;
	}

	AssetRecord newRecord = record;
	updateAssetInner(newRecord);
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

AssetHandle<Asset> Assets::bakeAssetFromResource(const ResourceWrapper<Resource>& resource, const std::string& name, const ScopedPath& targetDirectory)
{
	AssetType type = resource->getType();
	
	ResourceTypeManager* manager = AssetFactory::getManager(type);

	if (!manager)
	{
		logError("No ResourceTypeManager registered for asset type {}", static_cast<int>(type));
		return AssetHandle<Asset>::empty;
	}

	auto buildDesc = manager->makeResourceBuildDescriptor();
	if (!buildDesc)
	{
		logError("Invalid resource build descriptor");
		return AssetHandle<Asset>::empty;
	}

	manager->extractResourceData(resource, *buildDesc);

	AssetBuildDescriptor desc;
	desc.aType = type;
	desc.name = name;
	desc.isEngineOwned = targetDirectory.type() == ScopedPath::Type::Engine;
	desc.targetDirectory = targetDirectory;
	AssetHandle<Asset> asset = Engine::get()->getSubSystem<Assets>()->createAsset(desc, *buildDesc);

	if (asset.isEmpty())
	{
		logError("Failed to create asset from resource.");
		return AssetHandle<Asset>::empty;
	}

	return asset;
}

void Assets::renameAsset(UUID uuid, const std::string& newName)
{
	auto asset = getAsset(uuid);
	AssetRecord aInfo = asset.info();

	Engine::get()->getMemoryManagementSystem()->removePathReference(aInfo.getScopedPath().scoped().string());
	Engine::get()->getMemoryManagementSystem()->removeNameReference(aInfo.name);
	Engine::get()->getContext()->getProjectAssetRegistry()->removeAssetRegistry(aInfo);

	// Build new path (keep same directory + extension)
	std::filesystem::path oldPath = aInfo.getAbsolutePath();
	std::filesystem::path newPath = oldPath.parent_path() / (newName + oldPath.extension().string());

	// Rename on disk
	std::filesystem::rename(oldPath, newPath);

	aInfo.name = newName;
	updateAssetInner(aInfo);
}