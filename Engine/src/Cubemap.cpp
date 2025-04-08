#include "Cubemap.h"
#include "ApplicationConstants.h"

#include <GL/glew.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "Logger.h"
#include "Configurations.h"
#include "CacheSystem.h"
#include "Engine.h"
#include "Resource.h"
#include "Assets.h"
#include "Factory.h"
#include "Context.h"
#include "ProjectAssetRegistry.h"

#include "Texture.h" 

#include "EquirectangularToCubemapConverter.h"

Resource<Texture> Cubemap::createCubemapFromCubemapFiles(const std::vector<std::string>& faces)
{
	Texture::TextureData cubemapData = extractCubemapDataFromCubemapFiles(faces);
	Resource<Texture> cubemap = createCubemapFromBuffer(cubemapData);
	for (int i = 0; i < 6; i++)
	{
		stbi_image_free(cubemapData.facesData[i]);
	}
	return cubemap;
}

Texture::TextureData Cubemap::extractCubemapDataFromEquirectangularFile(const std::string& fileLocation)
{


	auto equirectangularMap = Texture::importTexture2D(fileLocation);
	

	Texture::TextureData cubemapData;
	cubemapData.target = GL_TEXTURE_CUBE_MAP;

	int width, height, nrChannels;
	cubemapData.facesData[0] = stbi_load(fileLocation.c_str(), &cubemapData.width, &cubemapData.height, &cubemapData.bpp, 0);

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

Resource<Texture> Cubemap::createCubemapFromBuffer(const Texture::TextureData& cubemapData)
{
	return build(cubemapData);
}

Resource<Texture> Cubemap::createDefaultCubemap()
{
	Texture::TextureData cubemapData;
	static unsigned char* FULL_WHITE = new unsigned char[3]{ 255, 255, 255 }; // todo fix
	for (int i = 0; i < 6; i++)
	{
		cubemapData.facesData[i] = FULL_WHITE;
	}
	cubemapData.target = GL_TEXTURE_CUBE_MAP;
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

Resource<Texture> Cubemap::createEmptyCubemap(int width, int height, int internalFormat, int format, int type)
{
	Texture::TextureData cubemapData;
	cubemapData.target = GL_TEXTURE_CUBE_MAP;
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

Resource<Texture> Cubemap::createEmptyCubemap(int width, int height, int internalFormat, int format, int type, std::map<int, int> params, bool createMipMaps)
{
	Texture::TextureData cubemapData;
	cubemapData.target = GL_TEXTURE_CUBE_MAP;
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
	cubemapData.target = GL_TEXTURE_CUBE_MAP;

	int width, height, nrChannels;
	for (unsigned int i = 0; i < files.size(); i++)
	{
		cubemapData.facesData[i] = stbi_load(files[i].c_str(), &cubemapData.width, &cubemapData.height, &cubemapData.bpp, 0);
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

Resource<Texture> Cubemap::build(const Texture::TextureData& textureData)
{
	Resource<Texture> texture = Factory<Texture>::create();

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

void Cubemap::saveEquirectangularMap(Resource<Texture> equirectangularMap)
{
	equirectangularMap.get()->bind();

	// Allocate memory for the pixels
	void* pixels = malloc(equirectangularMap.get()->getWidth() * equirectangularMap.get()->getHeight() * 3);

	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);

	auto& projectDir = Engine::get()->getProjectDirectory();
	std::string savedFilepath = projectDir + "/" + equirectangularMap.getUID() + ".png";
	stbi_write_png(savedFilepath.c_str(), equirectangularMap.get()->getWidth(), equirectangularMap.get()->getHeight(), 3, pixels,
		equirectangularMap.get()->getWidth() * 3);

	AssetInfo aInfo;
	aInfo.aType = AssetType::TEXTURE;
	aInfo.isValid = true;
	aInfo.filePath = savedFilepath;
	aInfo.uuid = equirectangularMap.getUID();
	Engine::get()->getSubSystem<Assets>()->addAsset(aInfo);
}

// adi loves you