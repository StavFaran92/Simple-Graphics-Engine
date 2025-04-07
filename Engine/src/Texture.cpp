#include "Texture.h"
#include "ApplicationConstants.h"

#include <GL/glew.h>

#include "Logger.h"
#include "Configurations.h"
#include "CacheSystem.h"
#include "Engine.h"
#include "Resource.h"
#include "Factory.h"
#include "Context.h"

#include "EquirectangularToCubemapConverter.h" // todo remove

//#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

//#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "Assets.h"

Texture::Texture()
	:m_id(0), m_slot(0)
{
}

Texture::Texture(const Texture& other)
	: m_id(other.m_id), m_slot(other.m_slot), m_data(other.m_data)
{
}

Resource<Texture> Texture::createEmptyTexture(int width, int height)
{
	return createEmptyTexture(width, height, GL_RGB, GL_RGB, GL_UNSIGNED_BYTE);
}

Resource<Texture> Texture::createEmptyTexture(int width, int height, int internalFormat, int format, int type)
{
	TextureData textureData;
	textureData.target = GL_TEXTURE_2D;
	textureData.width = width;
	textureData.height = height;
	textureData.internalFormat = (InternalFormat)internalFormat;
	textureData.format = (Format)format;
	textureData.type = (Type)type;
	textureData.params = {
		{GL_TEXTURE_MIN_FILTER, GL_NEAREST },
		{GL_TEXTURE_MAG_FILTER, GL_NEAREST },
	};

	return create2DTextureFromBuffer(textureData);
}

Resource<Texture> Texture::create2DTextureFromBuffer(const TextureData& textureData)
{
	Resource<Texture> texture = Factory<Texture>::create();

	texture.get()->build(textureData);

	return texture;
}

Resource<Texture> Texture::create2DTextureFromBuffer(int width, int height, int internalFormat, int format, int type, std::map<int, int> params, void* data)
{
	TextureData textureData;
	textureData.target = GL_TEXTURE_2D;
	textureData.width = width;
	textureData.height = height;
	textureData.bpp = 4;
	textureData.internalFormat = (InternalFormat)internalFormat;
	textureData.format = (Format)format;
	textureData.type = (Type)type;
	textureData.params = params;
	textureData.data = data;

	return create2DTextureFromBuffer(textureData);
}

Resource<Texture> Texture::createDummyTexture(unsigned char data[3])
{
	TextureData tData;
	tData.target = GL_TEXTURE_2D;
	tData.width = 1;
	tData.height = 1;
	tData.bpp = 3;
	tData.data = data;
	tData.internalFormat = InternalFormat::RGB2;
	tData.format = Format::RGB;
	tData.type = Type::UNSIGNED_BYTE;
	tData.params = { {GL_TEXTURE_MIN_FILTER, GL_LINEAR},
					{GL_TEXTURE_MAG_FILTER, GL_LINEAR},
					{GL_TEXTURE_WRAP_S, GL_REPEAT},
					{GL_TEXTURE_WRAP_T, GL_REPEAT } };

	return create2DTextureFromBuffer(tData);
}

void Texture::build(const TextureData& textureData)
{
	m_data = textureData;

	// generate texture
	glGenTextures(1, &m_id);
	bind();

	for (auto& [paramKey, paramValue] : textureData.params)
	{
		glTexParameteri(GL_TEXTURE_2D, paramKey, paramValue);
	}

	glTexImage2D(GL_TEXTURE_2D, 0, textureData.internalFormat, m_data.width, m_data.height, 0, textureData.format, (int)textureData.type, textureData.data);

	if (textureData.genMipMap)
	{
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	unbind();
}

int Texture::getWidth() const
{
	return m_data.width;
}

int Texture::getHeight() const
{
	return m_data.height;
}

int Texture::getBitDepth() const
{
	return m_data.bpp;
}

void Texture::setData(int xoffset, int yoffset, int width, int height, const void* data)
{
	bind();

	glTexSubImage2D(GL_TEXTURE_2D, 0, xoffset, yoffset, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
}

std::string Texture::textureTypeToString(TextureType type)
{
	switch (type)
	{
		case Texture::TextureType::Diffuse:
			return Constants::g_textureAlbedo;
		case Texture::TextureType::Specular:
			return Constants::g_textureSpecular;
		case Texture::TextureType::Albedo:
			return Constants::g_textureAlbedo;
		case Texture::TextureType::Normal:
			return Constants::g_textureNormal;
		case Texture::TextureType::Metallic:
			return Constants::g_textureMetallic;
		case Texture::TextureType::Roughness:
			return Constants::g_textureRoughness;
		case Texture::TextureType::AmbientOcclusion:
			return Constants::g_textureAO;

		default:
			logError("Unsupported texture format");
			return "";
	}
}

void Texture::bind() const
{
	glActiveTexture(GL_TEXTURE0 + m_slot);
	glBindTexture(m_data.target, m_id);
}

void Texture::unbind() const
{
	glActiveTexture(GL_TEXTURE0 + m_slot);
	glBindTexture(m_data.target, 0);
}

unsigned int Texture::getID() const
{
	return m_id;
}

void Texture::ClearTexture()
{
	glDeleteTextures(1, &m_id);
}

Texture::~Texture()
{
	logInfo( __FUNCTION__ );
	ClearTexture();
}

// Function to determine if the file is HDR based on its extension
bool isHDRImage(const std::string& filename) {
	return stbi_is_hdr(filename.c_str());
}

Texture::TextureData Texture::extractTextureDataFromFile(const std::string& fileLocation)
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

void Texture::writeTexture2D(const std::string& fileLocation, Resource<Texture> texture)
{
	stbi_write_png(fileLocation.c_str(),
		texture.get()->getWidth(),
		texture.get()->getHeight(),
		texture.get()->getBitDepth(),
		texture.get()->getData().data,
		texture.get()->getBitDepth());
}

Resource<Texture> Texture::importTexture2D(const std::string& fileLocation)
{
	
	Texture::TextureData textureData = extractTextureDataFromFile(fileLocation);
	Resource<Texture> texture = Texture::create2DTextureFromBuffer(textureData);

	AssetInfo aInfo;
	aInfo.origFilePath = fileLocation;
	aInfo.uuid = texture.getUID();
	aInfo.aType = AssetType::TEXTURE;
	Engine::get()->getSubSystem<Assets>()->importAsset(aInfo);

	//auto& projectDir = Engine::get()->getProjectDirectory();
	//if (textureData.isHDR)
	//{
	//	stbi_write_hdr((projectDir + "/" + texture.getUID() + ".hdr").c_str(), textureData.width, textureData.height, textureData.bpp, (float*)textureData.data);
	//}
	//else
	//{
	//	stbi_write_png((projectDir + "/" + texture.getUID() + ".png").c_str(), textureData.width, textureData.height, textureData.bpp, textureData.data, textureData.width * textureData.bpp);
	//}
	//Engine::get()->getContext()->getProjectAssetRegistry()->addTexture(texture);

	//stbi_image_free(textureData.data); // todo check if im not cleaning neede memory here

	return texture;
}

Resource<Texture> Texture::loadTexture2D(UUID uid, const std::string& path)
{
	Texture::TextureData textureData = extractTextureDataFromFile(path);

	// Create texture
	Texture* texture = new Texture();
	texture->build(textureData);
	Engine::get()->getMemoryPool<Texture>()->add(uid, texture);

	auto& res = Resource<Texture>(uid);


	return res;
}

void Texture::addTexture2D(Resource<Texture> texture)
{
	addTexture2D("Texture_" + texture.getUID().substr(4), texture);
}

void Texture::addTexture2D(const std::string& name, Resource<Texture> texture)
{
	AssetInfo aInfo;
	aInfo.aType = AssetType::TEXTURE;
	aInfo.uuid = texture.getUID();
	aInfo.name = name;
	aInfo.attributes["isHDR"] = "false";
	Engine::get()->getSubSystem<Assets>()->addAsset(aInfo);
}