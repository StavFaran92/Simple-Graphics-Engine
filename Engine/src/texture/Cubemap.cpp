#include "texture/Cubemap.h"
#include "core/ApplicationConstants.h"

#include <GL/glew.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "core/Logger.h"
#include "core/Configurations.h"
#include "core/CacheSystem.h"
#include "core/Engine.h"
#include "memory/ResourceWrapper.h"
#include "memory/Assets.h"
#include "core/Factory.h"
#include "runtime/Context.h"
#include "serialize/ProjectAssetRegistry.h"

#include "texture/Texture.h" 

#include "utils/EquirectangularToCubemapConverter.h"

ResourceWrapper<Texture> Cubemap::createCubemapFromCubemapFiles(const std::vector<std::string>& faces)
{
	Texture::TextureData cubemapData = extractCubemapDataFromCubemapFiles(faces);
	ResourceWrapper<Texture> cubemap = createCubemapFromBuffer(cubemapData);
	for (int i = 0; i < 6; i++)
	{
		stbi_image_free(cubemapData.facesData[i]);
	}
	return cubemap;
}

Texture::TextureData Cubemap::extractCubemapDataFromEquirectangularFile(const std::string& fileLocation)
{


	auto equirectangularMap = Texture::import(fileLocation);
	

	Texture::TextureData cubemapData;
	cubemapData.target = Texture::TextureTarget::TEXTURE_CUBE_MAP;

	int width, height, nrChannels;
	cubemapData.facesData[0] = stbi_load(fileLocation.c_str(), &cubemapData.width, &cubemapData.height, &cubemapData.channels, 0);

	cubemapData.format = (Texture::Format)GL_RGB;
	cubemapData.internalFormat = (Texture::InternalFormat)GL_RGB;
	cubemapData.type = (Texture::Type)GL_UNSIGNED_BYTE;
	cubemapData.params = {
		{ GL_TEXTURE_MIN_FILTER, GL_LINEAR},
		{ GL_TEXTURE_MAG_FILTER, GL_LINEAR},
		{ GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE},
		{ GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE},
		{ GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE},
	};

	cubemapData.genMipMap = false;

	return cubemapData;
	//return {};
}

//Resource<Texture> Cubemap::createCubemapFromEquirectangularFile(const std::string& fileLocation)
//{
//	//open equirect file
//		//create equirect texture
//	Texture::TextureData textureData = Texture::extractTextureDataFromFile(fileLocation);
//	Resource<Texture> equirectangularMap = Texture::create2DTextureFromBuffer(textureData);
//
//	//convert equirect to cubemap
//	auto cubemap = EquirectangularToCubemapConverter::fromEquirectangularToCubemap(equirectangularMap);
//
//	// todo use RAII
//	//CubemapData cubemapData = extractCubemapDataFromEquirectangularFile(fileLocation);
//
//	//Resource<Texture> cubemap = createCubemapFromBuffer(cubemapData);
//
//	equirectangularMap.get()->bind();
//
//	// Allocate memory for the pixels
//	void* pixels = malloc(equirectangularMap.get()->getWidth() * equirectangularMap.get()->getHeight() * equirectangularMap.get()->getBitDepth());
//
//	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);
//
//	auto& projectDir = Engine::get()->getProjectDirectory();
//	stbi_write_png((projectDir + "/" + cubemap.getUID() + ".png").c_str(), equirectangularMap.get()->getWidth(), equirectangularMap.get()->getHeight(), equirectangularMap.get()->getBitDepth(), pixels,
//		equirectangularMap.get()->getWidth() * equirectangularMap.get()->getBitDepth());
//
//	AssetInfo aInfo;
//	aInfo.aType = AssetType::TEXTURE;
//	aInfo.isValid = true;
//	aInfo.origFilePath = fileLocation;
//	aInfo.uuid = cubemap.getUID();
//	aInfo.attributes["cubemap"] = "True";
//	Engine::get()->getSubSystem<Assets>()->addAsset(aInfo);
//
//	
//
//	free(pixels);
//
//	return cubemap;
//}

ResourceWrapper<Texture> Cubemap::createCubemapFromBuffer(const Texture::TextureData& cubemapData)
{
	return build(cubemapData);
}

ResourceWrapper<Texture> Cubemap::createDefaultCubemap()
{
	Texture::TextureData cubemapData;
	static unsigned char* FULL_WHITE = new unsigned char[3]{ 255, 255, 255 }; // todo fix
	for (int i = 0; i < 6; i++)
	{
		cubemapData.facesData[i] = FULL_WHITE;
	}
	cubemapData.target = Texture::TextureTarget::TEXTURE_CUBE_MAP;
	cubemapData.width = 1;
	cubemapData.height = 1;
	cubemapData.internalFormat = Texture::InternalFormat::RGB2;
	cubemapData.format = Texture::Format::RGB;
	cubemapData.type = Texture::Type::UNSIGNED_BYTE;
	cubemapData.params = {
		{ GL_TEXTURE_MIN_FILTER, GL_LINEAR},
		{ GL_TEXTURE_MAG_FILTER, GL_LINEAR},
		{ GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE},
		{ GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE},
		{ GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE},
	};
	cubemapData.genMipMap = false;

	return createCubemapFromBuffer(cubemapData);
}

ResourceWrapper<Texture> Cubemap::createEmptyCubemap(int width, int height, int internalFormat, int format, int type)
{
	Texture::TextureData cubemapData;
	cubemapData.target = Texture::TextureTarget::TEXTURE_CUBE_MAP;
	cubemapData.width = width;
	cubemapData.height = height;
	cubemapData.internalFormat = (Texture::InternalFormat)internalFormat;
	cubemapData.format = (Texture::Format)format;
	cubemapData.type = (Texture::Type)type;
	cubemapData.params = {
		{ GL_TEXTURE_MIN_FILTER, GL_LINEAR},
		{ GL_TEXTURE_MAG_FILTER, GL_LINEAR},
		{ GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE},
		{ GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE},
		{ GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE},
	};
	cubemapData.genMipMap = false;

	return createCubemapFromBuffer(cubemapData);
}

ResourceWrapper<Texture> Cubemap::createEmptyCubemap(int width, int height, int internalFormat, int format, int type, std::map<int, int> params, bool createMipMaps)
{
	Texture::TextureData cubemapData;
	cubemapData.target = Texture::TextureTarget::TEXTURE_CUBE_MAP;
	cubemapData.width = width;
	cubemapData.height = height;
	cubemapData.internalFormat = (Texture::InternalFormat)internalFormat;
	cubemapData.format = (Texture::Format)format;
	cubemapData.type = (Texture::Type)type;
	cubemapData.params = params;
	cubemapData.genMipMap = createMipMaps;

	return createCubemapFromBuffer(cubemapData);
}

Texture::TextureData Cubemap::extractCubemapDataFromCubemapFiles(const std::vector<std::string>& files)
{
	Texture::TextureData cubemapData;
	cubemapData.target = Texture::TextureTarget::TEXTURE_CUBE_MAP;

	int width, height, nrChannels;
	for (unsigned int i = 0; i < files.size(); i++)
	{
		cubemapData.facesData[i] = stbi_load(files[i].c_str(), &cubemapData.width, &cubemapData.height, &cubemapData.channels, 0);
	}

	cubemapData.format = (Texture::Format)GL_RGB;
	cubemapData.internalFormat = (Texture::InternalFormat)GL_RGB;
	cubemapData.type = (Texture::Type)GL_UNSIGNED_BYTE;

	cubemapData.params = {
		{ GL_TEXTURE_MIN_FILTER, GL_LINEAR},
		{ GL_TEXTURE_MAG_FILTER, GL_LINEAR},
		{ GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE},
		{ GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE},
		{ GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE},
	};

	cubemapData.genMipMap = false;

	return cubemapData;
}

ResourceWrapper<Texture> Cubemap::build(const Texture::TextureData& textureData)
{
	ResourceWrapper<Texture> texture = Factory<Texture>::create();

	texture.get()->m_data = textureData;

	// generate texture
	glGenTextures(1, &texture.get()->m_id);
	texture.get()->bind();

	for (auto& [paramKey, paramValue] : textureData.params)
	{
		glTexParameteri(GL_TEXTURE_CUBE_MAP, paramKey, paramValue);
	}

	for (int i = 0; i < 6; i++)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, textureData.internalFormat, texture.get()->m_data.width, texture.get()->m_data.height, 0, textureData.format, textureData.type, textureData.facesData[i]);
	}
	if (textureData.genMipMap)
	{
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	}

	texture.get()->unbind();

	return texture;
}

void Cubemap::saveEquirectangularMap(ResourceWrapper<Texture> equirectangularMap)
{
	equirectangularMap.get()->bind();

	// Allocate memory for the pixels
	void* pixels = malloc(equirectangularMap.get()->getWidth() * equirectangularMap.get()->getHeight() * 3);

	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);

	equirectangularMap->m_data.data = pixels;
	equirectangularMap->m_data.channels = 3;

	AssetCreateDescriptor aInfo;
	aInfo.aType = AssetType::TEXTURE;
	Engine::get()->getSubSystem<Assets>()->createAsset(equirectangularMap, aInfo);
}

// adi loves you