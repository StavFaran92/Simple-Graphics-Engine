#include "Assets.h"

#include <GL/glew.h>


#include "Texture.h"
#include "Animation.h"
#include "AnimationLoader.h"
#include "CacheSystem.h"
#include "ModelImporter.h"
#include "Factory.h"

#include <filesystem>

Assets::Assets()
{
	Engine::get()->registerSubSystem<Assets>(this);
}

//ModelImporter::ModelInfo Assets::importMesh(const std::string& fileLocation)
//{
//	auto modelInfo = Engine::get()->getSubSystem<ModelImporter>()->import(fileLocation);
//	AssetInfo aInfo;
//	aInfo.uuid = modelInfo.mesh.getUID();
//	aInfo.origFilePath = fileLocation;
//	aInfo.aType = AssetType::MESH;
//	importAsset(aInfo);
//	//m_meshes[modelInfo.mesh.getUID()] = modelInfo.mesh;
//	return modelInfo;
//}

std::vector<std::string> Assets::getAllMeshes() const
{
	std::vector<std::string> result;
	for (auto [uuid, _] : m_meshes)
	{
		result.push_back(uuid);
	}
	return result;
}

std::vector<std::string> Assets::getAllTextures() const
{
	std::vector<std::string> result;
	for (auto [uuid, _] : m_textures)
	{
		result.push_back(uuid);
	}
	return result;
}

Resource<Animation> Assets::importAnimation(const std::string& fileLocation)
{
	auto memoryManagementSystem = Engine::get()->getMemoryManagementSystem();
	std::filesystem::path path(fileLocation);
	return memoryManagementSystem->createOrGetCached<Animation>(path.filename().string(), [&]() {

		auto animation = Engine::get()->getSubSystem<AnimationLoader>()->import(fileLocation);

		Engine::get()->getContext()->getProjectAssetRegistry()->addAnimation(animation.getUID());

		m_animations[animation.getUID()] = animation;

		return animation;
		});
}

Resource<Animation> Assets::loadAnimation(UUID uid, const std::string& path)
{
	return Resource<Animation>();
}

std::vector<std::string> Assets::getAllAnimations() const
{
	std::vector<std::string> result;
	for (auto [uuid, _] : m_animations)
	{
		result.push_back(uuid);
	}
	return result;
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

	m_assets[aType].insert(uid);

	aInfo.filePath = savedFilePath;
	aInfo.isValid = true;

	Engine::get()->getContext()->getProjectAssetRegistry()->addAssetRegistry(aInfo);

	logInfo("Successfully imported asset: '" + path + "' into: '" + savedFilePath + "'.");

	return aInfo;
}

AssetInfo Assets::addAsset(AssetInfo aInfo)
{
	auto& path = aInfo.origFilePath;
	auto& uid = aInfo.uuid;
	auto& aType = aInfo.aType;
	auto& savedFilepath = aInfo.filePath;
	
	Engine::get()->getMemoryManagementSystem()->addAssociation(aInfo.name, uid); //TODO maybe use some naming convention here?

	m_assets[aType].insert(uid);

	aInfo.filePath = savedFilepath;
	aInfo.isValid = true;
	aInfo.aType = aType;

	Engine::get()->getContext()->getProjectAssetRegistry()->addAssetRegistry(aInfo);

	logInfo("Successfully Added asset: '" + savedFilepath + "'.");

	return aInfo;
}

std::vector<std::string> Assets::getAllAssetsOfType(AssetType aType) const
{
	std::vector<std::string> result;
	for (auto uuid : m_assets.at(aType))
	{
		result.push_back(uuid);
	}
	return result;
}

std::string Assets::getAlias(UUID uid) const
{
	return Engine::get()->getMemoryManagementSystem()->getName(uid);
}
