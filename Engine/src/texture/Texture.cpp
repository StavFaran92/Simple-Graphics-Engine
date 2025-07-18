#include "texture/Texture.h"
#include "core/ApplicationConstants.h"

#include <GL/glew.h>

#include "core/Logger.h"
#include "core/Configurations.h"
#include "core/CacheSystem.h"
#include "core/Engine.h"
#include "memory/Resource.h"
#include "core/Factory.h"
#include "runtime/Context.h"

#include "utils/EquirectangularToCubemapConverter.h" // todo remove

//#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

//#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "memory/Assets.h"

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
		{GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE},
		{GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE}
	};

	return create2DTextureFromBuffer(textureData);
}

Resource<Texture> Texture::create2DTextureFromBuffer(const TextureData& textureData)
{
	Resource<Texture> texture;
	if (textureData.isTransient)
	{
		if (textureData.textureName.empty())
		{
			logError("Transient texture must have a name!");
			return Resource<Texture>::empty;
		}
		texture = Factory<Texture>::createUsingCustomUUID(textureData.textureName);
	}
	else
	{
		texture = Factory<Texture>::create();
	}

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

	m_attributes.flip = textureData.flip;
	m_attributes.genMipMap = textureData.genMipMap;
	m_attributes.isHDR = textureData.isHDR;

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

Texture::TextureAssetAttributes Texture::getTextureAssetAttributes()
{
	return m_attributes;;
}

void Texture::extractTextureDataFromFile(const std::string& fileLocation, Texture::TextureData& textureData)
{
	// Determine if the image is HDR
	if (isHDRImage(fileLocation))
	{
		textureData.isHDR = true;
	}

	stbi_set_flip_vertically_on_load(textureData.flip);

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

	std::string textureName = std::filesystem::path(fileLocation).filename().stem().string();
	textureData.textureName = textureName;

	textureData.type = (textureData.isHDR) ? (Texture::Type)GL_FLOAT : (Texture::Type)GL_UNSIGNED_BYTE;
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

Resource<Texture> Texture::importTexture2D(const std::string& fileLocation, const TextureImportSettings& settings)
{
	Texture::TextureData textureData;

	textureData.target = GL_TEXTURE_2D;
	
	// extract texture build data
	extractTextureDataFromSettings(settings, textureData);
	extractTextureDataFromFile(fileLocation, textureData);
	Resource<Texture> texture = Texture::create2DTextureFromBuffer(textureData);

	if (!settings.isTransient)
	{
		AssetInfo aInfo;
		aInfo.origFilePath = fileLocation;
		aInfo.uuid = texture.getUID();
		aInfo.aType = AssetType::TEXTURE;
		aInfo.isTransient = textureData.isTransient;

		if (!settings.name.empty())
		{
			aInfo.name = settings.name;
		}
		else
		{
			aInfo.name = std::filesystem::path(fileLocation).filename().stem().string();

		}

		aInfo.attributes = texture->getTextureAssetAttributes().toMap();
		Engine::get()->getSubSystem<Assets>()->importAsset(aInfo);
	}

	return texture;
}

Resource<Texture> Texture::loadTexture2D(AssetInfo aInfo)
{
	Texture::TextureData textureData;

	textureData.target = GL_TEXTURE_2D;

	// extract texture build data
	TextureAssetAttributes attributes(aInfo.attributes);
	extractTextureDataFromAttributes(attributes, textureData);
	const std::string filepath = Engine::get()->getProjectDirectory() + aInfo.filePath;
	extractTextureDataFromFile(filepath, textureData);

	// Create texture
	Texture* texture = new Texture();
	texture->build(textureData);
	Engine::get()->getMemoryPool<Texture>()->add(aInfo.uuid, texture);

	texture->m_attributes = attributes;

	auto& res = Resource<Texture>(aInfo.uuid);


	return res;
}

void Texture::addTexture2D(Resource<Texture> texture)
{
	addTexture2D("Texture_" + texture.getUID().substr(4), texture);
}

void Texture::addTexture2D(const std::string& name, Resource<Texture> texture)
{
	texture.get()->bind();

	// Allocate memory for the pixels
	void* pixels = malloc(texture.get()->getWidth() * texture.get()->getHeight() * 3);

	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);

	texture->m_data.data = pixels;
	texture->m_data.bpp = 3;

	auto& projectDir = Engine::get()->getProjectDirectory();
	const std::string relativeFilepath = "/" + texture.getUID() + ".png";
	std::string savedFileLocation = projectDir + relativeFilepath;
	writeTexture2D(savedFileLocation, texture);

	AssetInfo aInfo;
	aInfo.aType = AssetType::TEXTURE;
	aInfo.uuid = texture.getUID();
	aInfo.name = name;
	aInfo.filePath = relativeFilepath;
	aInfo.attributes = texture->getTextureAssetAttributes().toMap();
	Engine::get()->getSubSystem<Assets>()->addAsset(aInfo);
}

void Texture::extractTextureDataFromSettings(const TextureImportSettings& settings, Texture::TextureData& textureData)
{
	textureData.params = settings.params;

	// if params not specified use default params values
	if (textureData.params.empty())
	{
		if (settings.genMipMap)
		{
			textureData.params = {
				{ GL_TEXTURE_WRAP_S, GL_REPEAT},
				{ GL_TEXTURE_WRAP_T, GL_REPEAT},
				{ GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR},
				{ GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR},
			};
		}
		else
		{
			textureData.params = {
				{ GL_TEXTURE_WRAP_S, GL_REPEAT},
				{ GL_TEXTURE_WRAP_T, GL_REPEAT},
				{ GL_TEXTURE_WRAP_R, GL_REPEAT},
				{ GL_TEXTURE_MIN_FILTER, GL_LINEAR},
				{ GL_TEXTURE_MAG_FILTER, GL_LINEAR},
			};
		}
	}

	textureData.genMipMap = settings.genMipMap;
	textureData.flip = settings.flip;
	textureData.isTransient = settings.isTransient;
	
}

void Texture::extractTextureDataFromAttributes(const TextureAssetAttributes& attributes, Texture::TextureData& textureData)
{
	if (attributes.genMipMap)
	{
		textureData.params = {
			{ GL_TEXTURE_WRAP_S, GL_REPEAT},
			{ GL_TEXTURE_WRAP_T, GL_REPEAT},
			{ GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR},
			{ GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR},
		};

		textureData.genMipMap = true;
	}
	else
	{
		textureData.params = {
			{ GL_TEXTURE_WRAP_S, GL_REPEAT},
			{ GL_TEXTURE_WRAP_T, GL_REPEAT},
			{ GL_TEXTURE_WRAP_R, GL_REPEAT},
			{ GL_TEXTURE_MIN_FILTER, GL_LINEAR},
			{ GL_TEXTURE_MAG_FILTER, GL_LINEAR},
		};

		textureData.genMipMap = false;
	}

	textureData.flip = attributes.flip;
}

Resource<Texture> Texture::importTexture3D(const std::string& fileLocation)
{
	Texture::TextureData textureData;

	textureData.target = GL_TEXTURE_3D;

	Texture::TextureImportSettings settings;
	settings.name = "VolumeCloud";

	// extract texture build data
	extractTextureDataFromSettings(settings, textureData);
	extractTextureDataFromFile(fileLocation, textureData);
	
	Resource<Texture> texture = Factory<Texture>::create();

	texture.get()->m_data = textureData;

	texture.get()->m_attributes.flip = textureData.flip;
	texture.get()->m_attributes.genMipMap = textureData.genMipMap;
	texture.get()->m_attributes.isHDR = textureData.isHDR;

	// generate texture
	glGenTextures(1, &texture.get()->m_id);
	texture.get()->bind();

	for (auto& [paramKey, paramValue] : textureData.params)
	{
		glTexParameteri(textureData.target, paramKey, paramValue);
	}

	int sliceSize = 64;
	int slicesPerRow = 512 / sliceSize;

	std::vector<unsigned char> volumeData(sliceSize * sliceSize * 64 * 4); // RGBA
	for (int z = 0; z < 64; ++z) {
		int tileX = z % slicesPerRow;
		int tileY = z / slicesPerRow;

		for (int y = 0; y < sliceSize; ++y) {
			for (int x = 0; x < sliceSize; ++x) {
				int srcX = tileX * sliceSize + x;
				int srcY = tileY * sliceSize + y;
				int srcIndex = (srcY * 512 + srcX) * 4;
				int dstIndex = ((z * sliceSize + y) * sliceSize + x) * 4;

				volumeData[dstIndex + 0] = ((uint8_t*)(textureData.data))[srcIndex + 0];
				volumeData[dstIndex + 1] = ((uint8_t*)(textureData.data))[srcIndex + 1];
				volumeData[dstIndex + 2] = ((uint8_t*)(textureData.data))[srcIndex + 2];
				volumeData[dstIndex + 3] = ((uint8_t*)(textureData.data))[srcIndex + 3];
			}
		}
	}

	glTexImage3D(textureData.target, 0, GL_RGBA8, 64, 64, 64, 0, GL_RGBA, GL_UNSIGNED_BYTE, volumeData.data());

	if (textureData.genMipMap)
	{
		glGenerateMipmap(textureData.target);
	}

	texture.get()->unbind();

	return texture;
}

//adi is your love of your life