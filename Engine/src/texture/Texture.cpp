#include "texture/Texture.h"
#include "core/ApplicationConstants.h"

#include <GL/glew.h>

#include "core/Logger.h"
#include "core/Configurations.h"
#include "core/Engine.h"
#include "memory/ResourceWrapper.h"
#include "core/Factory.h"

#include "memory/Assets.h"

#include "memory/AssetFactory.h"

#include "utils/EXRLoader.h"

#include "utils/STBIHelper.h"

#include "utils/TextureUtils.h"

GLint TextureWrapToOpenGL(Texture::TextureWrap wrap)
{
	switch (wrap)
	{
	case Texture::TextureWrap::Repeat: return GL_REPEAT;
	case Texture::TextureWrap::Clamp:  return GL_CLAMP_TO_EDGE;
	case Texture::TextureWrap::Mirror: return GL_MIRRORED_REPEAT;
	default: return GL_CLAMP_TO_EDGE;
	}
}

GLint TextureFilterToOpenGL(Texture::TextureFilter filter, bool hasMipmaps, bool isMinFilter)
{
	if (filter == Texture::TextureFilter::Nearest)
	{
		if (isMinFilter && hasMipmaps)
			return GL_NEAREST_MIPMAP_NEAREST;
		else
			return GL_NEAREST;
	}

	// Linear
	if (isMinFilter && hasMipmaps)
		return GL_LINEAR_MIPMAP_LINEAR;
	else
		return GL_LINEAR;
}

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

	assert(textureData.data);

	// Create texture resource
	ResourceWrapper<Texture> texture = Factory<Texture>::create();
	texture->build(textureData);

	return texture;
}

std::string TextureAssetManager::getRecommendedExtension(const AssetInfo& aInfo)
{

	Texture::TextureAssetDescriptor settings{};

	if (aInfo.importSettings.is_object() && !aInfo.importSettings.empty())
	{
		settings = aInfo.importSettings.get<Texture::TextureAssetDescriptor>();

		if (settings.usage == Texture::TextureSemantic::Heightmap ||
			settings.usage == Texture::TextureSemantic::Environment ||
			settings.usage == Texture::TextureSemantic::LUT ||
			settings.usage == Texture::TextureSemantic::Data)
		{
			return ".exr";
		}
		else if (settings.usage == Texture::TextureSemantic::Color ||
			settings.usage == Texture::TextureSemantic::Normal||
			settings.usage == Texture::TextureSemantic::Mask)
		{
			return ".png";
		}
	}

	return ".png";


}

void TextureAssetManager::save(const AssetWrapper<ResourceBase>& texture, const AssetInfo& aInfo)
{
	auto projectDir = Engine::get()->getProjectDirectory();
	std::string fileLocation = projectDir + "/" + aInfo.relativefilePath;
	auto& resource = texture.as<Texture>().resource();

	if (resource.get()->getData().type == Texture::Type::FLOAT)
	{
		EXRLoader::saveSingleChannelEXR(fileLocation, resource.get()->getWidth(),
			resource.get()->getHeight(),
			(const float*)resource.get()->getData().data);
	}
	else
	{
		STBIHelper::writeToPNG(fileLocation,
			resource.get()->getWidth(),
			resource.get()->getHeight(),
			resource.get()->getChannels(),
			resource.get()->getData().data,
			resource.get()->getWidth() * resource.get()->getChannels());

		
	}
}

Texture::Texture()
	:m_id(0), m_slot(0)
{}

void Texture::setTextureParameters(const Texture::TextureData& tData)
{
	GLenum targetGL = toGL(tData.target);

	// Filtering
	glTexParameteri(targetGL, GL_TEXTURE_MIN_FILTER, TextureFilterToOpenGL(tData.filter, tData.genMipMap, true));
	glTexParameteri(targetGL, GL_TEXTURE_MAG_FILTER, TextureFilterToOpenGL(tData.filter, tData.genMipMap, false));

	// Wrapping - all dimensions default to same wrap mode
	glTexParameteri(targetGL, GL_TEXTURE_WRAP_S, TextureWrapToOpenGL(tData.wrap));
	glTexParameteri(targetGL, GL_TEXTURE_WRAP_T, TextureWrapToOpenGL(tData.wrap));

	if (tData.target == TextureTarget::TEXTURE_3D || tData.target == TextureTarget::TEXTURE_CUBE_MAP)
	{
		glTexParameteri(targetGL, GL_TEXTURE_WRAP_R, TextureWrapToOpenGL(tData.wrap));
	}
}

void Texture::fillTextureBufferIfNeeded(TextureData& tData)
{
	if (!tData.data)
	{
		if (tData.target == Texture::TextureTarget::TEXTURE_2D)
		{
			tData.data = TextureUtils::createBlankTextureBuffer2D(
				tData.width,
				tData.height,
				tData.format,
				tData.type);
		}
		else if (tData.target == Texture::TextureTarget::TEXTURE_3D)
		{
			tData.data = TextureUtils::createBlankTextureBuffer3D(
				tData.width,
				tData.height,
				tData.depth,
				tData.format,
				tData.type);
		}
		else if (tData.target == Texture::TextureTarget::TEXTURE_CUBE_MAP)
		{
			for (int i = 0; i < 6; i++)
			{
				tData.facesData[i] = TextureUtils::createBlankTextureBuffer2D(
					tData.width,
					tData.height,
					tData.format,
					tData.type);
			}
		}
		else 
		{
			logError("Texture target not supported.");
			return;
		}
	}
}

ResourceWrapper<Texture> Texture::createTexture(TextureData& textureData)
{
	ResourceWrapper<Texture> texture = Factory<Texture>::create();

	if (textureData.fillEmpty)
	{
		fillTextureBufferIfNeeded(textureData);
	}
	texture.get()->build(textureData);
	return texture;
}

ResourceWrapper<Texture> Texture::createTexture(int width, int height, int channels, InternalFormat internalFormat, Format format, Type type, TextureFilter filter, TextureWrap wrap, void* data)
{
	TextureData textureData;
	textureData.target = Texture::TextureTarget::TEXTURE_2D;
	textureData.width = width;
	textureData.height = height;
	textureData.channels = channels;
	textureData.internalFormat = internalFormat;
	textureData.format = format;
	textureData.type = type;
	textureData.filter = filter;
	textureData.wrap = wrap;
	textureData.data = data;

	return createTexture(textureData);
}

ResourceWrapper<Texture> Texture::createTexture(int width, int height, Texture::TextureSemantic usage, void* data)
{
	InternalFormat internalFormat = getInternalFormatFromUsage(usage);
	Format format = Format::RGBA;
	Type type = Type::UNSIGNED_BYTE;
	TextureFilter filter = TextureFilter::Linear;
	TextureWrap wrap = TextureWrap::Clamp;
	int channels = 3;

	switch (usage)
	{
	case TextureSemantic::Color:
		format = Format::RGBA;
		type = Type::UNSIGNED_BYTE;
		channels = 3;
		break;

	case TextureSemantic::Normal:
		format = Format::RGB;
		type = Type::UNSIGNED_BYTE;
		channels = 3;
		break;

	case TextureSemantic::Mask:
		format = Format::RED;
		type = Type::UNSIGNED_BYTE;
		channels = 1;
		break;

	case TextureSemantic::Heightmap:
		format = Format::RED;
		type = Type::FLOAT;
		channels = 1;
		break;

	case TextureSemantic::Data:
		format = Format::RGBA;
		type = Type::FLOAT;
		channels = 4;
		break;

	case TextureSemantic::Environment:
		format = Format::RGB;
		type = Type::FLOAT;
		channels = 3;
		break;

	case TextureSemantic::LUT:
		format = Format::RGB;
		type = Type::UNSIGNED_BYTE;
		channels = 3;
		break;

	default:
		format = Format::RGBA;
		type = Type::UNSIGNED_BYTE;
		channels = 3;
		break;
	}

	return Texture::createTexture(width, height, channels, internalFormat, format, type, filter, wrap, data);
}

void Texture::build(const TextureData& textureData)
{
	m_data = textureData;

	// generate texture
	glGenTextures(1, &m_id);
	bind();

	setTextureParameters(textureData);

	if (textureData.target == Texture::TextureTarget::TEXTURE_2D)
	{
		//glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexImage2D(toGL(textureData.target),
			0,
			toGL(textureData.internalFormat),
			m_data.width,
			m_data.height,
			0,
			toGL(textureData.format),
			toGL(textureData.type),
			textureData.data);
	}
	else if (textureData.target == Texture::TextureTarget::TEXTURE_CUBE_MAP)
	{
		for (int i = 0; i < 6; i++)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
				0, 
				toGL(textureData.internalFormat), 
				m_data.width, 
				m_data.height, 0, 
				toGL(textureData.format),
				toGL(textureData.type),
				textureData.facesData[i]);
		}
	}
	else if (textureData.target == Texture::TextureTarget::TEXTURE_3D)
	{
		glTexImage3D(toGL(textureData.target), 
			0, 
			toGL(textureData.internalFormat),
			m_data.width,
			m_data.height, 
			m_data.depth,
			0, 
			toGL(textureData.format),
			toGL(textureData.type),
			textureData.data);
	}
	else {
		logError("Unsupported texture format.");
		return;
	}

	if (textureData.genMipMap)
	{
		glGenerateMipmap(toGL(textureData.target));
	}
}

int Texture::getWidth() const
{
	return m_data.width;
}

int Texture::getHeight() const
{
	return m_data.height;
}

int Texture::getChannels() const
{
	return m_data.channels;
}

void Texture::setData(int xoffset, int yoffset, int width, int height, const void* data)
{
	bind();

	glTexSubImage2D(GL_TEXTURE_2D, 0, xoffset, yoffset, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
}

void Texture::generateMipMaps()
{
	bind();
	glGenerateMipmap(toGL(m_data.target));
}

void Texture::bind() const
{
	glActiveTexture(GL_TEXTURE0 + m_slot);
	glBindTexture(toGL(m_data.target), m_id);
}

void Texture::unbind() const
{
	glActiveTexture(GL_TEXTURE0 + m_slot);
	glBindTexture(toGL(m_data.target), 0);
}

unsigned int Texture::getID() const
{
	return m_id;
}

void Texture::ClearTexture()
{
	glDeleteTextures(1, &m_id);

	if (m_data.data)
		free(m_data.data);

	for (int i = 0; i < 6; i++)
	{
		if (m_data.facesData[i]) free(m_data.facesData[i]);
	}
}

Texture::~Texture()
{
	ClearTexture();
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

ResourceWrapper<Texture> Texture::clone() const
{
	TextureData newTextureData(m_data);

	const uint32_t bpp = TextureUtils::channelCount(m_data.format) * TextureUtils::bytesPerChannel(m_data.type);
	const size_t size = static_cast<size_t>(m_data.width) * m_data.height * bpp;

	// Calculate buffer size
	const size_t bufferSize = size;

	// Allocate new buffer
	newTextureData.data = std::malloc(bufferSize);
	if (!newTextureData.data)
	{
		logError("Texture clone: allocation failed.");
		return ResourceWrapper<Texture>::empty;
	}

	// Copy raw pixel data
	std::memcpy(newTextureData.data, m_data.data, bufferSize);

	//if (m_data.target == Texture::TextureTarget::TEXTURE_2D)
	//{
	//	newTextureData.data = TextureUtils::createBlankTextureBuffer2D(m_data.width, m_data.height, m_data.format, m_data.type);
	//}
	//else if (m_data.target == Texture::TextureTarget::TEXTURE_3D)
	//{
	//	newTextureData.data = TextureUtils::createBlankTextureBuffer3D(m_data.width, m_data.height, m_data.depth, m_data.format, m_data.type);
	//}
	//else
	//{
	//	logError("Not yet implemented.");
	//	return ResourceWrapper<Texture>::empty;
	//}

	ResourceWrapper<Texture> clonedTexture = Texture::createTexture(newTextureData);

	if (clonedTexture.isEmpty())
	{
		logWarning("Texture clone failed.");
	}

	return clonedTexture;
}

Texture::Format Texture::getFormatFromChannels(int channels)
{
	if (channels == 1) return Texture::Format::RED;
	else if (channels == 3) return Texture::Format::RGB;
	else if (channels == 4) return Texture::Format::RGBA;
	else {
		logError("Unsupported texture format!");
		return Texture::Format::RGB;
	}
}

Texture::InternalFormat Texture::getInternalFormatFromUsage(Texture::TextureSemantic usage)
{
	switch (usage)
	{
	case Texture::TextureSemantic::Color:
		return Texture::InternalFormat::RGBA;

	case Texture::TextureSemantic::Normal:
		return Texture::InternalFormat::RGB16F;

	case Texture::TextureSemantic::Heightmap:
		return Texture::InternalFormat::R32F;

	case Texture::TextureSemantic::Mask:
		return Texture::InternalFormat::R8UI;

	case Texture::TextureSemantic::Data:
		return Texture::InternalFormat::RGBA32F;

	case Texture::TextureSemantic::Environment:
		return Texture::InternalFormat::RGB16F;
	}

	return Texture::InternalFormat::RGB;
}

void Texture::extractTextureDataFromSettings(const TextureAssetDescriptor& settings, Texture::TextureData& textureData)
{
	textureData.filter = settings.filter;
	textureData.wrap = settings.wrap;
	textureData.genMipMap = settings.genMipMap;
	textureData.flip = settings.flip;
	textureData.internalFormat = getInternalFormatFromUsage(settings.usage);
}



//ResourceWrapper<Texture> Texture::importTexture3D(const std::string& fileLocation)
//{
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
	//return {};
//}

void Texture::extractTextureDataFromFile(const std::string& fileLocation, Texture::TextureData& textureData)
{
	// flip if needed
	STBIHelper::setFlip(textureData.flip);

	std::filesystem::path p(fileLocation);
	std::string ext = p.extension().string();

	if (ext == ".exr")
	{
		EXRLoader::loadSingleChannelEXR(fileLocation, textureData.width, textureData.height, textureData.data);
		textureData.type = Texture::Type::FLOAT;
		textureData.channels = 1;

		//InspectEXRChannels(fileLocation.c_str());
	}
	else if (STBIHelper::isHDR(fileLocation.c_str()))
	{
		textureData.data = STBIHelper::loadImageFloat(fileLocation, &textureData.width, &textureData.height, &textureData.channels);
		textureData.type = Texture::Type::FLOAT;

		float* pixels = static_cast<float*>(textureData.data);

		bool detectedOverflowRadianceValues = false;
		for (int i = 0; i < textureData.width * textureData.height * textureData.channels; ++i) {
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
		textureData.data = STBIHelper::loadImage(fileLocation, &textureData.width, &textureData.height, &textureData.channels);
		textureData.type = Texture::Type::UNSIGNED_BYTE;
	}

	// load validation
	if (!textureData.data)
	{
		logError("Failed to load file: {}", fileLocation);
	}

	textureData.format = getFormatFromChannels(textureData.channels);

	std::string textureName = std::filesystem::path(fileLocation).filename().stem().string();
	textureData.textureName = textureName;
}

//adi is your love of your life
