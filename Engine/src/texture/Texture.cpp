#include "texture/Texture.h"
#include "core/ApplicationConstants.h"

#include <GL/glew.h>

#include "core/Logger.h"
#include "core/Configurations.h"
#include "core/CacheSystem.h"
#include "core/Engine.h"
#include "memory/ResourceWrapper.h"
#include "core/Factory.h"
#include "runtime/Context.h"

#include "utils/EquirectangularToCubemapConverter.h" // todo remove

//#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

//#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "memory/Assets.h"

#include "memory/AssetFactory.h"

#include "memory/AssetLoader.h"

namespace {
	struct TextureManagerRegistration {
		TextureManagerRegistration() {
			AssetFactory::registerManager(AssetType::TEXTURE, std::make_shared<TextureAssetManager>());
		}
	} _textureManagerRegistration;
}

bool TextureAssetManager::copyFiles(const std::string& fileLocation, AssetInfo& aInfo)
{
	const std::filesystem::path projectDir = Engine::get()->getProjectDirectory();
	const std::filesystem::path savedFilePath = projectDir / aInfo.relativefilePath;
	return std::filesystem::copy_file(fileLocation, savedFilePath, std::filesystem::copy_options::overwrite_existing);
}

ResourceWrapper<ResourceBase> TextureAssetManager::load(AssetInfo& aInfo)
{
	std::string filepath = aInfo.fullFilePath;

	Texture::TextureData textureData;

	// extract texture build data
	Texture::TextureAssetDescriptor settings{};

	if (aInfo.importSettings.is_object() && !aInfo.importSettings.empty())
	{
		settings = aInfo.importSettings.get<Texture::TextureAssetDescriptor>();
	}
	Texture::extractTextureDataFromSettings(settings, textureData);
	Texture::extractTextureDataFromFile(filepath, textureData);

	// Create texture resource
	ResourceWrapper<Texture> texture = Factory<Texture>::create();
	texture->build(textureData);

	return texture;
}

void TextureAssetManager::save(const AssetWrapper<ResourceBase>& texture, const AssetInfo& aInfo)
{
	auto projectDir = Engine::get()->getProjectDirectory();
	std::string fileLocation = projectDir + "/" + aInfo.relativefilePath;

	Texture::writeTexture2D(fileLocation, texture.as<Texture>().resource());
}

Texture::Texture()
	:m_id(0), m_slot(0)
{
}

Texture::Texture(const Texture& other)
	: m_id(other.m_id), m_slot(other.m_slot), m_data(other.m_data)
{
}

ResourceWrapper<Texture> Texture::createEmptyTexture(int width, int height)
{
	return createEmptyTexture(width, height, GL_RGB, GL_RGB, GL_UNSIGNED_BYTE);
}

ResourceWrapper<Texture> Texture::createEmptyTexture(int width, int height, int internalFormat, int format, int type)
{
	TextureData textureData;
	textureData.target = Texture::TextureTarget::TEXTURE_2D;
	textureData.width = width;
	textureData.height = height;
	textureData.internalFormat = (InternalFormat)internalFormat;
	textureData.format = (Format)format;
	textureData.type = (Type)type;
	textureData.params = {
		{GL_TEXTURE_MIN_FILTER, GL_LINEAR },
		{GL_TEXTURE_MAG_FILTER, GL_LINEAR },
		{GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE},
		{GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE}
	};

	return create2DTextureFromBuffer(textureData);
}

ResourceWrapper<Texture> Texture::createEmptyTexture(int width, int height, int internalFormat, int format, int type, std::map<int, int> params)
{
	TextureData textureData;
	textureData.target = Texture::TextureTarget::TEXTURE_2D;
	textureData.width = width;
	textureData.height = height;
	textureData.internalFormat = (InternalFormat)internalFormat;
	textureData.format = (Format)format;
	textureData.type = (Type)type;
	textureData.params = params;

	return create2DTextureFromBuffer(textureData);
}

ResourceWrapper<Texture> Texture::create2DTextureFromBuffer(const TextureData& textureData)
{
	ResourceWrapper<Texture> texture;
	//UUID uuid = textureData.textureName.empty() ? UUID::generate_uuid_v4() : textureData.textureName;
	texture = Factory<Texture>::create();
	texture.get()->build(textureData);

	return texture;
}

ResourceWrapper<Texture> Texture::create2DTextureFromBuffer(int width, int height, int internalFormat, int format, int type, std::map<int, int> params, bool isEngineOwned, void* data)
{
	TextureData textureData;
	textureData.target = Texture::TextureTarget::TEXTURE_2D;
	textureData.width = width;
	textureData.height = height;
	textureData.bpp = 4;
	textureData.internalFormat = (InternalFormat)internalFormat;
	textureData.format = (Format)format;
	textureData.type = (Type)type;
	textureData.params = params;
	textureData.isEngineOwned = isEngineOwned;
	textureData.data = data;

	return create2DTextureFromBuffer(textureData);
}

void Texture::build(const TextureData& textureData)
{
	m_data = textureData;

	m_attributes.flip = textureData.flip;
	m_attributes.genMipMap = textureData.genMipMap;
	m_attributes.isHDR = textureData.isHDR;
	m_attributes.params = textureData.params;

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

void Texture::generateMipMaps()
{
	bind();
	glGenerateMipmap(m_data.target);
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
	ClearTexture();
}



Texture::TextureAssetAttributes Texture::getTextureAssetAttributes()
{
	return m_attributes;;
}

void Texture::writeTexture2D(const std::string& fileLocation, ResourceWrapper<Texture> texture)
{
	stbi_write_png(fileLocation.c_str(),
		texture.get()->getWidth(),
		texture.get()->getHeight(),
		texture.get()->getBitDepth(),
		texture.get()->getData().data,
		texture.get()->getWidth() * texture.get()->getBitDepth());
}

AssetWrapper<Texture> Texture::import(const std::string& fileLocation, TextureAssetDescriptor desc)
{
	desc.aType = AssetType::TEXTURE;
	return Engine::get()->getSubSystem<Assets>()->importAsset(fileLocation, desc).as<Texture>();
}

ResourceWrapper<Texture> Texture::load(const std::string& fileLocation, TextureAssetDescriptor desc)
{
	desc.aType = AssetType::TEXTURE;
	return Engine::get()->getSubSystem<Assets>()->loadResource(fileLocation, desc).as<Texture>();
}

void Texture::addTexture2D(ResourceWrapper<Texture> texture)
{
	addTexture2D("Texture_" + std::to_string(texture.getUID()).substr(4), texture);
}

void Texture::addTexture2D(const std::string& name, ResourceWrapper<Texture> texture)
{
	texture.get()->bind();

	// Allocate memory for the pixels
	void* pixels = malloc(texture.get()->getWidth() * texture.get()->getHeight() * 3);

	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);

	texture->m_data.data = pixels;
	texture->m_data.bpp = 3;

	AssetCreateDescriptor aInfo;
	aInfo.aType = AssetType::TEXTURE;
	aInfo.name = name;
	aInfo.attributes = texture->getTextureAssetAttributes().toMap();
	Engine::get()->getSubSystem<Assets>()->createAsset(texture, aInfo);
}

void Texture::extractTextureDataFromSettings(const TextureAssetDescriptor& settings, Texture::TextureData& textureData)
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
	textureData.internalFormat = settings.GPUformat;
	//textureData.isTransient = settings.isTransient;
	
}

unsigned char* Texture::decodeCompressedFromMemory(const unsigned char* rawBuffer, int len, int* outWidth, int* outHeight, int* outChannels) 
{
	auto buffer = stbi_load_from_memory(rawBuffer, len, outWidth, outHeight, outChannels, 3);
	return buffer;
}

ResourceWrapper<Texture> Texture::importTexture3D(const std::string& fileLocation)
{
	// TODO fix

	//Texture::TextureData textureData;

	//textureData.target = GL_TEXTURE_3D;

	//Texture::TextureImportSettings settings;
	//settings.name = "VolumeCloud";

	//// extract texture build data
	//extractTextureDataFromSettings(settings, textureData);
	//extractTextureDataFromFile(fileLocation, textureData);
	//
	//Resource<Texture> texture = Factory<Texture>::create();

	//texture.get()->m_data = textureData;

	//texture.get()->m_attributes.flip = textureData.flip;
	//texture.get()->m_attributes.genMipMap = textureData.genMipMap;
	//texture.get()->m_attributes.isHDR = textureData.isHDR;

	//// generate texture
	//glGenTextures(1, &texture.get()->m_id);
	//texture.get()->bind();

	//for (auto& [paramKey, paramValue] : textureData.params)
	//{
	//	glTexParameteri(textureData.target, paramKey, paramValue);
	//}

	//int sliceSize = 64;
	//int slicesPerRow = 512 / sliceSize;

	//std::vector<unsigned char> volumeData(sliceSize * sliceSize * 64 * 4); // RGBA
	//for (int z = 0; z < 64; ++z) {
	//	int tileX = z % slicesPerRow;
	//	int tileY = z / slicesPerRow;

	//	for (int y = 0; y < sliceSize; ++y) {
	//		for (int x = 0; x < sliceSize; ++x) {
	//			int srcX = tileX * sliceSize + x;
	//			int srcY = tileY * sliceSize + y;
	//			int srcIndex = (srcY * 512 + srcX) * 4;
	//			int dstIndex = ((z * sliceSize + y) * sliceSize + x) * 4;

	//			volumeData[dstIndex + 0] = ((uint8_t*)(textureData.data))[srcIndex + 0];
	//			volumeData[dstIndex + 1] = ((uint8_t*)(textureData.data))[srcIndex + 1];
	//			volumeData[dstIndex + 2] = ((uint8_t*)(textureData.data))[srcIndex + 2];
	//			volumeData[dstIndex + 3] = ((uint8_t*)(textureData.data))[srcIndex + 3];
	//		}
	//	}
	//}

	//glTexImage3D(textureData.target, 0, GL_RGBA8, 64, 64, 64, 0, GL_RGBA, GL_UNSIGNED_BYTE, volumeData.data());

	//if (textureData.genMipMap)
	//{
	//	glGenerateMipmap(textureData.target);
	//}

	//texture.get()->unbind();

	//return texture;
	return {};
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

		float* pixels = static_cast<float*>(textureData.data);

		bool detectedOverflowRadianceValues = false;
		for (int i = 0; i < textureData.width * textureData.height * textureData.bpp; ++i) {
			if (!std::isfinite(pixels[i]) || std::abs(pixels[i]) > HALF_MAX)
			{
				pixels[i] = std::clamp(pixels[i], -HALF_MAX, HALF_MAX);
				detectedOverflowRadianceValues = true;

			}
		}

		if (detectedOverflowRadianceValues)
		{
			logWarning("Deteced texture values that are above half max, clamping to half max.");
		}
	}
	else
	{
		textureData.data = stbi_load(fileLocation.c_str(), &textureData.width, &textureData.height, &textureData.bpp, 0);
	}

	// load validation
	if (!textureData.data)
	{
		logError("Failed to load file: {}", fileLocation);
	}

	// Determine format based on bits per pixel (bpp)
	if (textureData.bpp == 1)
	{
		textureData.format = (Texture::Format)GL_RED;
		//textureData.internalFormat = (Texture::InternalFormat)((textureData.isHDR) ? GL_R16F : GL_R8); // HDR: 16-bit float, Non-HDR: 8-bit
	}
	else if (textureData.bpp == 3)
	{
		textureData.format = (Texture::Format)GL_RGB;
		//textureData.internalFormat = (Texture::InternalFormat)((textureData.isHDR) ? GL_RGB16F : GL_RGB8); // HDR: 16-bit float, Non-HDR: 8-bit
	}
	else if (textureData.bpp == 4)
	{
		textureData.format = (Texture::Format)GL_RGBA;
		//textureData.internalFormat = (Texture::InternalFormat)((textureData.isHDR) ? GL_RGBA16F : GL_RGBA8); // HDR: 16-bit float, Non-HDR: 8-bit
	}
	else {
		logError("Unsupported texture format!");
		return;
	}

	std::string textureName = std::filesystem::path(fileLocation).filename().stem().string();
	textureData.textureName = textureName;

	textureData.type = (textureData.isHDR) ? (Texture::Type)GL_FLOAT : (Texture::Type)GL_UNSIGNED_BYTE;
}

// Function to determine if the file is HDR based on its extension
bool Texture::isHDRImage(const std::string& filename) {
	return stbi_is_hdr(filename.c_str());
}

//adi is your love of your life
