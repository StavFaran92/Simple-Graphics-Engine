#include "Assets.h"

#include <GL/glew.h>


#include "Texture.h"
#include "Animation.h"
#include "AnimationLoader.h"
#include "CacheSystem.h"
#include "ModelImporter.h"
#include "Factory.h"

//#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

//#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <filesystem>

Assets::Assets()
{
	Engine::get()->registerSubSystem<Assets>(this);
}

ModelImporter::ModelInfo Assets::importMesh(const std::string& fileLocation)
{
	auto modelInfo = Engine::get()->getSubSystem<ModelImporter>()->import(fileLocation);
	AssetInfo aInfo;
	aInfo.uuid = modelInfo.mesh.getUID();
	aInfo.origFilePath = fileLocation;
	aInfo.aType = AssetType::MESH;
	importAsset(aInfo);
	//m_meshes[modelInfo.mesh.getUID()] = modelInfo.mesh;
	return modelInfo;
}

std::vector<std::string> Assets::getAllMeshes() const
{
	std::vector<std::string> result;
	for (auto [uuid, _] : m_meshes)
	{
		result.push_back(uuid);
	}
	return result;
}

// Function to determine if the file is HDR based on its extension
bool isHDRImage(const std::string& filename) {
	return stbi_is_hdr(filename.c_str());
}

Texture::TextureData Assets::extractTextureDataFromFile(const std::string& fileLocation)
{
	Texture::TextureData textureData;
	textureData.target = GL_TEXTURE_2D;

	// Determine if the image is HDR
	if (isHDRImage(fileLocation))
	{
		textureData.isHDR = true;
	}

	if (textureData.isHDR)
	{
		textureData.data = stbi_loadf(fileLocation.c_str(), &textureData.width, &textureData.height, &textureData.bpp, 0);
	}
	else
	{
		textureData.data = stbi_load(fileLocation.c_str(), &textureData.width, &textureData.height, &textureData.bpp, 0);
	}

	

	// load validation
	if (!textureData.data)
	{
		logError("Failed to find: {}", fileLocation.c_str());
		return {};
	}

	// Determine format based on bits per pixel (bpp)
	if (textureData.bpp == 1)
	{
		textureData.format = (Texture::Format)GL_RED;
		textureData.internalFormat = (Texture::InternalFormat)((textureData.isHDR) ? GL_R16F : GL_R8); // HDR: 16-bit float, Non-HDR: 8-bit
	}
	else if (textureData.bpp == 3) 
	{
		textureData.format = (Texture::Format)GL_RGB;
		textureData.internalFormat = (Texture::InternalFormat)((textureData.isHDR) ? GL_RGB16F : GL_RGB8); // HDR: 16-bit float, Non-HDR: 8-bit
	}
	else if (textureData.bpp == 4) 
	{
		textureData.format = (Texture::Format)GL_RGBA;
		textureData.internalFormat = (Texture::InternalFormat)((textureData.isHDR) ? GL_RGBA16F : GL_RGBA8); // HDR: 16-bit float, Non-HDR: 8-bit
	}
	else {
		throw std::runtime_error("Unsupported texture format!");
	}

	textureData.type = (textureData.isHDR) ? (Texture::Type)GL_FLOAT : (Texture::Type)GL_UNSIGNED_BYTE;

	textureData.params = {
		{ GL_TEXTURE_WRAP_S, GL_REPEAT},
		{ GL_TEXTURE_WRAP_T, GL_REPEAT},
		{ GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR},
		{ GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR},
	};

	textureData.genMipMap = true; // todo fix we dont always want to generate mipmaps for loaded textures

	return textureData;
}

Resource<Texture> Assets::importTexture2D(const std::string& assetName, std::function<Resource<Texture>()> func)
{
	return Engine::get()->getMemoryManagementSystem()->createOrGetCached<Texture>(assetName, [&]() {
			Resource<Texture> res = func();
			Engine::get()->getContext()->getProjectAssetRegistry()->addTexture(res);
			auto& projectDir = Engine::get()->getProjectDirectory();
			stbi_write_png((projectDir + "/" + res.getUID() + ".png").c_str(), 
				res.get()->getWidth(), 
				res.get()->getHeight(), 
				res.get()->getBitDepth(), 
				res.get()->getData().data, 
				res.get()->getBitDepth());
			m_textures[res.getUID()] = res;
			return res;
		} );
}

Resource<Texture> Assets::importTexture2D(const std::string& fileLocation, bool flip)
{
	// Check if texture is already cached to optimize the load process
	auto memoryManagementSystem = Engine::get()->getMemoryManagementSystem();
	std::filesystem::path path(fileLocation);
	return memoryManagementSystem->createOrGetCached<Texture>(path.filename().string(), [&]() {

		// todo use RAII
		//stbi_set_flip_vertically_on_load(flip);
		Texture::TextureData textureData = extractTextureDataFromFile(fileLocation);

		Resource<Texture> texture = Texture::create2DTextureFromBuffer(textureData);

		auto& projectDir = Engine::get()->getProjectDirectory();
		if (textureData.isHDR)
		{
			stbi_write_hdr((projectDir + "/" + texture.getUID() + ".hdr").c_str(), textureData.width, textureData.height, textureData.bpp, (float*)textureData.data);
		}
		else
		{
			stbi_write_png((projectDir + "/" + texture.getUID() + ".png").c_str(), textureData.width, textureData.height, textureData.bpp, textureData.data, textureData.width * textureData.bpp);
		}
		Engine::get()->getContext()->getProjectAssetRegistry()->addTexture(texture);

		m_textures[texture.getUID()] = texture;

		//stbi_image_free(textureData.data); // todo check if im not cleaning neede memory here

		return texture;
		});
}

Resource<Texture> Assets::loadTexture2D(UUID uid, const std::string& path)
{
	Texture::TextureData textureData = extractTextureDataFromFile(path);

	// Create texture
	Texture* texture = new Texture();
	texture->build(textureData);
	Engine::get()->getMemoryPool<Texture>()->add(uid, texture);

	auto& res = Resource<Texture>(uid);

	m_textures[uid] = res;

	return res;
}

void Assets::addTexture2D(Resource<Texture> texture)
{
	addTexture2D("Texture_" + texture.getUID().substr(4), texture);
}

void Assets::addTexture2D(const std::string& name, Resource<Texture> texture)
{
	Engine::get()->getContext()->getProjectAssetRegistry()->addTexture(texture);
	Engine::get()->getMemoryManagementSystem()->addAssociation(name, texture.getUID());
	m_textures[texture.getUID()] = texture;
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

	logInfo("Successfully imported asset: [" + path + "] into: [" + savedFilePath + "].");

	return aInfo;
}

AssetInfo Assets::addAsset(AssetInfo assetInfo)
{
	auto& path = assetInfo.origFilePath;
	auto& uid = assetInfo.uuid;
	auto& aType = assetInfo.aType;
	auto& savedFilepath = assetInfo.filePath;
	
	//Engine::get()->getMemoryManagementSystem()->addAssociation(path, uid); //TODO maybe use some naming convention here?

	m_assets[aType].insert(uid);

	AssetInfo aInfo;
	aInfo.filePath = savedFilepath;
	aInfo.isValid = true;
	aInfo.aType = aType;

	Engine::get()->getContext()->getProjectAssetRegistry()->addAssetRegistry(aInfo);

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
