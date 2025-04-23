#include "Assets.h"

#include <GL/glew.h>


#include "Texture.h"
#include "Animation.h"
#include "AnimationLoader.h"
#include "CacheSystem.h"
#include "ModelImporter.h"
#include "Factory.h"
#include "ShaderBuilder.h"

#include <filesystem>

Assets::Assets()
{
	m_assets = {};
	//m_assets[AssetType::MESH] = {};
	//m_assets[AssetType::ANIMATION] = {};
	//m_assets[AssetType::TEXTURE] = {};
	//m_assets[AssetType::SHADER] = {};
	Engine::get()->registerSubSystem<Assets>(this);
}

//Resource<Animation> Assets::importAnimation(const std::string& fileLocation)
//{
//	auto memoryManagementSystem = Engine::get()->getMemoryManagementSystem();
//	std::filesystem::path path(fileLocation);
//	return memoryManagementSystem->createOrGetCached<Animation>(path.filename().string(), [&]() {
//
//		auto animation = Engine::get()->getSubSystem<AnimationLoader>()->import(fileLocation);
//
//		Engine::get()->getContext()->getProjectAssetRegistry()->addAnimation(animation.getUID());
//
//		m_animations[animation.getUID()] = animation;
//
//		return animation;
//		});
//}
//
//Resource<Animation> Assets::loadAnimation(UUID uid, const std::string& path)
//{
//	return Resource<Animation>();
//}
//
//std::vector<std::string> Assets::getAllAnimations() const
//{
//	std::vector<std::string> result;
//	for (auto [uuid, _] : m_animations)
//	{
//		result.push_back(uuid);
//	}
//	return result;
//}



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
	Engine::get()->getMemoryPool<T>()->add(uid, res.get());

	AssetType aType = getAssetType<T>();
	m_assets[aType].insert(uid);

	return res;
}

AssetInfo Assets::importAsset(AssetInfo aInfo)
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

	std::string fullName = std::filesystem::path(path).filename().string();
	std::string name = fullName.substr(0, fullName.find_first_of('.'));
	std::string ext = std::filesystem::path(path).extension().string();

	// Save asset in resource folder
	auto& projectDir = Engine::get()->getProjectDirectory();
	const std::string savedFilePath = projectDir + "/" + uid + ext;
	std::filesystem::copy_file(path, savedFilePath);

	
	Engine::get()->getMemoryManagementSystem()->addAssociation(path, uid);

	aInfo.filePath = savedFilePath;
	aInfo.isValid = true;

	Engine::get()->getContext()->getProjectAssetRegistry()->addAssetRegistry(aInfo);

	m_assets[uid] = aInfo;

	logInfo("Successfully imported asset: '" + path + "' into: '" + savedFilePath + "'.");

	return aInfo;
}

AssetInfo Assets::addAsset(AssetInfo aInfo)
{
	auto& path = aInfo.origFilePath;
	auto& uid = aInfo.uuid;
	auto& aType = aInfo.aType;
	auto& savedFilepath = aInfo.filePath;

	assert(!savedFilepath.empty());
	assert(!uid.empty());
	
	Engine::get()->getMemoryManagementSystem()->addAssociation(aInfo.name, uid); //TODO maybe use some naming convention here?

	aInfo.filePath = savedFilepath;
	aInfo.isValid = true;
	aInfo.aType = aType;

	Engine::get()->getContext()->getProjectAssetRegistry()->addAssetRegistry(aInfo);

	m_assets[uid] = aInfo;

	logInfo("Successfully Added asset: '" + savedFilepath + "'.");

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

void Assets::load()
{
	auto par = Engine::get()->getContext()->getProjectAssetRegistry();

	// Load meshes
	std::vector<AssetInfo> meshAssets = par->getAllAssetsOfType(AssetType::MESH);
	for (const auto& asset : meshAssets)
	{
		UUID uuid = asset.uuid;
		ModelImporter::ModelInfo mInfo;
		MeshCollection* meshPtr = new MeshCollection();
		Resource<MeshCollection> generatedMesh(uuid);
		mInfo.mesh = generatedMesh;
		Engine::get()->getMemoryPool<MeshCollection>()->add(uuid, meshPtr);
		Engine::get()->getResourceManager()->incRef(uuid);
		Engine::get()->getSubSystem<ModelImporter>()->load(asset.filePath, mInfo);
		m_assets[uuid] = asset;
	}

	// Load textures
	std::vector<AssetInfo> textureAssets = par->getAllAssetsOfType(AssetType::TEXTURE);
	for (const auto& asset : textureAssets)
	{
		UUID uuid = asset.uuid;
		Engine::get()->getResourceManager()->incRef(uuid);
		Texture::loadTexture2D(asset);
		m_assets[uuid] = asset;
	}

	// Load animations
	std::vector<AssetInfo> animationNameList = par->getAllAssetsOfType(AssetType::ANIMATION);
	for (const auto& asset : animationNameList)
	{
		UUID uuid = asset.uuid;
		Animation* animPtr = new Animation();
		Resource<Animation> anim(uuid);
		Engine::get()->getMemoryPool<Animation>()->add(uuid, animPtr);
		Engine::get()->getResourceManager()->incRef(uuid);
		Engine::get()->getSubSystem<AnimationLoader>()->load(asset.filePath, anim);
		m_assets[uuid] = asset;
	}

	// Load Shaders
	std::vector<AssetInfo> shaderAssets = par->getAllAssetsOfType(AssetType::SHADER);
	for (const auto& asset : shaderAssets)
	{
		UUID uuid = asset.uuid;
		std::string shaderOverrideStr = asset.attributes.at("shader_override");
		ShaderOverride shaderOverride = Shader::getShaderOverrideFromStr(shaderOverrideStr);
		Shader* shaderPtr = new Shader();
		Resource<Shader> shader(uuid);
		Engine::get()->getMemoryPool<Shader>()->add(uuid, shaderPtr);
		Engine::get()->getResourceManager()->incRef(uuid);
		Shader::load(shader, asset.filePath, shaderOverride);

		m_assets[uuid] = asset;
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
