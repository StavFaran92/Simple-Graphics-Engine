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

//template<typename T>
//Assets::AssetType Assets::getAssetType()
//{
//	if constexpr (std::is_same_v<T, Mesh>) return AssetType::MESH;
//	if constexpr (std::is_same_v<T, Texture>) return AssetType::TEXTURE;
//	if constexpr (std::is_same_v<T, Animation>) return AssetType::ANIMATION;
//	if constexpr (std::is_same_v<T, Shader>) return AssetType::SHADER;
//	return AssetType::NONE;
//}
//
template<typename T>
Resource<T> Assets::loadAsset(UUID uid, const std::string& path)
{
	auto& res = Resource<T>(uid);

	// add asset to memory pool
	Engine::get()->getMemoryPool().add(uid, res.get());

	AssetType aType = getAssetType<T>();
	m_assets[aType].insert(uid);

	return res;
}

AssetInfo Assets::importAsset(AssetInfo& aInfo)
{
	auto& path = aInfo.origFilePath;
	auto& uid = aInfo.uuid;
	auto& aType = aInfo.aType;

	// Validate
	if (!std::filesystem::exists(path))
	{
		logError("File doesn't exists: " + path);
		return {};
	}

	const auto& allAssetsOfType = getAllAssetsOfType(aInfo.aType);
	for (const auto& asset : allAssetsOfType)
	{
		// Should add override option in settings
		if (asset.name == aInfo.name)
		{
			logWarning("Asset name must be unique, abort asset import");
			return {};
		}
	}

	std::string fullName = std::filesystem::path(path).filename().string();
	std::string name = fullName.substr(0, fullName.find_first_of('.'));
	std::string ext = std::filesystem::path(path).extension().string();

	
	

	if (!aInfo.isTransient)
	{
		// Save asset in resource folder
		auto& projectDir = Engine::get()->getProjectDirectory();
		const std::string relativeFilepath = "/" + fullName;
		const std::string savedFilePath = projectDir + relativeFilepath;
		std::filesystem::copy_file(path, savedFilePath);

		aInfo.filePath = relativeFilepath;

		Engine::get()->getMemoryManagementSystem()->addAssociation(fullName, uid);
		Engine::get()->getContext()->getProjectAssetRegistry()->addAssetRegistry(aInfo);
	}

	aInfo.isValid = true;

	m_assets[name] = aInfo;

	logInfo("Successfully imported asset: '" + path + "' into: '" + aInfo.name + "'.");

	return aInfo;
}

AssetInfo Assets::addAsset(AssetInfo& aInfo)
{
	auto& path = aInfo.origFilePath;
	auto& uid = aInfo.uuid;
	auto& aType = aInfo.aType;
	auto& savedFilepath = aInfo.filePath;

	assert(!savedFilepath.empty());
	assert(!uid.empty());
	

	aInfo.filePath = savedFilepath;
	aInfo.isValid = true;
	aInfo.aType = aType;

	if (!aInfo.isTransient)
	{
		Engine::get()->getMemoryManagementSystem()->addAssociation(aInfo.name, uid); //TODO maybe use some naming convention here?
		Engine::get()->getContext()->getProjectAssetRegistry()->addAssetRegistry(aInfo);
	}

	m_assets[aInfo.name] = aInfo;

	logInfo("Successfully Added asset: '" + aInfo.name + "'.");

	return aInfo;
}

std::vector<AssetInfo> Assets::getAllAssetsOfType(AssetType aType) const
{
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

void Assets::load()
{
	auto par = Engine::get()->getContext()->getProjectAssetRegistry();

	std::vector<AssetInfo> assets = par->getAllAssets();

	for (auto& assetInfo : assets)
	{
		AssetFactory::loadAsset(assetInfo);
		m_assets[assetInfo.name] = assetInfo;
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

std::string Assets::getAlias(UUID uid) const
{
	auto iter = m_assets.find(uid);
	if (iter != m_assets.end())
	{
		return iter->second.name;
	}
	return "N/A";

}

AssetInfo Assets::updateAsset(AssetInfo& aInfo)
{
	if (!aInfo.isTransient)
	{
		Engine::get()->getContext()->getProjectAssetRegistry()->updateAssetRegistry(aInfo);
	}

	m_assets[aInfo.name] = aInfo;

	logInfo("Successfully Updated asset: '" + aInfo.name + "'.");

	return aInfo;
}