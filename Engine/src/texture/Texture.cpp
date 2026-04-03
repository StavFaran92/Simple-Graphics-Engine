#include "texture/Texture.h"
#include "core/ApplicationConstants.h"

#include <GL/glew.h>

#include "core/Logger.h"
#include "core/Configurations.h"
#include "core/Engine.h"
#include "memory/ResourceWrapper.h"
#include "core/Factory.h"

#include "utils/EXRLoader.h"

#include "utils/STBIHelper.h"

#include "utils/TextureUtils.h"

GLint TextureWrapToOpenGL(TextureWrap wrap)
{
	switch (wrap)
	{
	case TextureWrap::Repeat: return GL_REPEAT;
	case TextureWrap::Clamp:  return GL_CLAMP_TO_EDGE;
	case TextureWrap::Mirror: return GL_MIRRORED_REPEAT;
	default: return GL_CLAMP_TO_EDGE;
	}
}

GLint TextureFilterToOpenGL(TextureFilter filter, bool hasMipmaps, bool isMinFilter)
{
	if (filter == TextureFilter::Nearest)
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


Texture::Texture()
	:m_id(0), m_slot(0)
{}

void Texture::setTextureParameters(const TextureData& tData)
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
		if (tData.target == TextureTarget::TEXTURE_2D)
		{
			tData.data = TextureUtils::createBlankTextureBuffer2D(
				tData.width,
				tData.height,
				tData.format,
				tData.type);
		}
		else if (tData.target == TextureTarget::TEXTURE_3D)
		{
			tData.data = TextureUtils::createBlankTextureBuffer3D(
				tData.width,
				tData.height,
				tData.depth,
				tData.format,
				tData.type);
		}
		else if (tData.target == TextureTarget::TEXTURE_CUBE_MAP)
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

void Texture::copyBufferIntoInternalBuffer(void*& data, size_t bufferSize)
{
	if (!data)
	{
		logError("Texture data is null.");
		return;
	}

	void* newBuffer = std::malloc(bufferSize);
	if (!newBuffer)
	{
		logError("Texture buffer allocation failed.");
		return;
	}

	// COPY FROM OLD -> NEW
	std::memcpy(newBuffer, data, bufferSize);

	// Redirect pointer
	data = newBuffer;
}

ResourceWrapper<Texture> Texture::createTexture(TextureData& textureData)
{
	ResourceWrapper<Texture> texture = Factory<Texture>::create();

	const uint32_t bytesPerPixel =
		TextureUtils::channelCount(textureData.format) *
		TextureUtils::bytesPerChannel(textureData.type);

	const size_t bufferSize =
		static_cast<size_t>(textureData.width) *
		static_cast<size_t>(textureData.height) *
		static_cast<size_t>(std::max(1, textureData.depth)) *
		bytesPerPixel;

	if (textureData.data)
	{
		copyBufferIntoInternalBuffer(textureData.data, bufferSize);
	}
	if (textureData.facesData[0])
	{
		for (int i = 0; i < 6; i++)
		{
			copyBufferIntoInternalBuffer(textureData.facesData[i], bufferSize);
		}
	}
	else if (textureData.fillEmpty)
	{
		fillTextureBufferIfNeeded(textureData);
	}
	texture.get()->build(textureData);
	return texture;
}

ResourceWrapper<Texture> Texture::createTexture(int width, int height, int channels, TextureInternalFormat internalFormat, TextureFormat format, TextureType type, TextureFilter filter, TextureWrap wrap, void* data)
{
	TextureData textureData;
	textureData.target = TextureTarget::TEXTURE_2D;
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

ResourceWrapper<Texture> Texture::createTexture(int width, int height, TextureSemantic usage, void* data)
{
	TextureInternalFormat internalFormat = getInternalFormatFromUsage(usage);
	TextureFormat format = TextureFormat::RGBA;
	TextureType type = TextureType::UNSIGNED_BYTE;
	TextureFilter filter = TextureFilter::Linear;
	TextureWrap wrap = TextureWrap::Clamp;
	int channels = 3;

	switch (usage)
	{
	case TextureSemantic::Color:
		format = TextureFormat::RGBA;
		type = TextureType::UNSIGNED_BYTE;
		channels = 3;
		break;

	case TextureSemantic::Normal:
		format = TextureFormat::RGB;
		type = TextureType::UNSIGNED_BYTE;
		channels = 3;
		break;

	case TextureSemantic::Mask:
		format = TextureFormat::RED;
		type = TextureType::UNSIGNED_BYTE;
		channels = 1;
		break;

	case TextureSemantic::Heightmap:
		format = TextureFormat::RED;
		type = TextureType::FLOAT;
		channels = 1;
		break;

	case TextureSemantic::Data:
		format = TextureFormat::RGBA;
		type = TextureType::FLOAT;
		channels = 4;
		break;

	case TextureSemantic::Environment:
		format = TextureFormat::RGB;
		type = TextureType::FLOAT;
		channels = 3;
		break;

	case TextureSemantic::LUT:
		format = TextureFormat::RGB;
		type = TextureType::UNSIGNED_BYTE;
		channels = 3;
		break;

	default:
		format = TextureFormat::RGBA;
		type = TextureType::UNSIGNED_BYTE;
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

	if (textureData.target == TextureTarget::TEXTURE_2D)
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
	else if (textureData.target == TextureTarget::TEXTURE_CUBE_MAP)
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
	else if (textureData.target == TextureTarget::TEXTURE_3D)
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
	m_data.genMipMap = true;
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

bool Texture::download()
{
	bind();

	const GLenum target = toGL(m_data.target);
	const GLenum format = toGL(m_data.format);
	const GLenum type   = toGL(m_data.type);

	const uint32_t bytesPerPixel =
		TextureUtils::channelCount(m_data.format) *
		TextureUtils::bytesPerChannel(m_data.type);

	if (m_data.target == TextureTarget::TEXTURE_2D)
	{
		const size_t size =
			static_cast<size_t>(m_data.width) *
			static_cast<size_t>(m_data.height) *
			bytesPerPixel;

		if (m_data.data) { free(m_data.data); m_data.data = nullptr; }
		m_data.data = std::malloc(size);
		if (!m_data.data) { logError("Texture::download allocation failed (2D)"); return false; }

		glGetTexImage(target, 0, format, type, m_data.data);
		return true;
	}
	else if (m_data.target == TextureTarget::TEXTURE_3D)
	{
		const size_t size =
			static_cast<size_t>(m_data.width) *
			static_cast<size_t>(m_data.height) *
			static_cast<size_t>(std::max(1, m_data.depth)) *
			bytesPerPixel;

		if (m_data.data) { free(m_data.data); m_data.data = nullptr; }
		m_data.data = std::malloc(size);
		if (!m_data.data) { logError("Texture::download allocation failed (3D)"); return false; }

		// glGetTexImage works for 3D textures as well
		glGetTexImage(target, 0, format, type, m_data.data);
		return true;
	}
	else if (m_data.target == TextureTarget::TEXTURE_CUBE_MAP)
	{
		// Allocate and fetch each face
		const size_t faceSize =
			static_cast<size_t>(m_data.width) *
			static_cast<size_t>(m_data.height) *
			bytesPerPixel;

		for (int i = 0; i < 6; ++i)
		{
			if (m_data.facesData[i]) { free(m_data.facesData[i]); m_data.facesData[i] = nullptr; }
			m_data.facesData[i] = std::malloc(faceSize);
			if (!m_data.facesData[i]) { logError("Texture::download allocation failed (Cubemap face {})", i); return false; }

			glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, type, m_data.facesData[i]);
		}
		return true;
	}

	logError("Texture::download unsupported texture target");
	return false;
}

void Texture::ClearTexture()
{
	glDeleteTextures(1, &m_id);

	if (m_data.data)
		free(m_data.data);

	for (int i = 0; i < 6; i++)
	{
		if (m_data.facesData[i]) 
			free(m_data.facesData[i]);
	}
}

Texture::~Texture()
{
	ClearTexture();
}

ResourceWrapper<Texture> Texture::load(const std::string& fileLocation, TextureLoadDescriptor desc)
{
	std::string filepath = fileLocation;

	TextureData textureData;

	// extract texture build data
	Texture::extractTextureDataFromSettings(desc, textureData);
	Texture::extractTextureDataFromFile(filepath, textureData);

	assert(textureData.data);

	// Create texture resource
	ResourceWrapper<Texture> texture = Factory<Texture>::create();
	texture->build(textureData);

	return texture;
}

ResourceWrapper<Texture> Texture::clone() const
{
	if (m_data.target == TextureTarget::TEXTURE_3D)
	{
		logError("Clone is not supported for 3D textures");
		return ResourceWrapper<Texture>::empty;
	}

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

	ResourceWrapper<Texture> clonedTexture = Texture::createTexture(newTextureData);

	if (clonedTexture.isEmpty())
	{
		logWarning("Texture clone failed.");
	}

	return clonedTexture;
}

TextureFormat Texture::getFormatFromChannels(int channels)
{
	if (channels == 1) return TextureFormat::RED;
	else if (channels == 3) return TextureFormat::RGB;
	else if (channels == 4) return TextureFormat::RGBA;
	else {
		logError("Unsupported texture format!");
		return TextureFormat::RGB;
	}
}

TextureInternalFormat Texture::getInternalFormatFromUsage(TextureSemantic usage)
{
	switch (usage)
	{
	case TextureSemantic::Color:
		return TextureInternalFormat::RGBA;

	case TextureSemantic::Normal:
		return TextureInternalFormat::RGB16F;

	case TextureSemantic::Heightmap:
		return TextureInternalFormat::R32F;

	case TextureSemantic::Mask:
		return TextureInternalFormat::R8UI;

	case TextureSemantic::Data:
		return TextureInternalFormat::RGBA32F;

	case TextureSemantic::Environment:
		return TextureInternalFormat::RGB16F;
	}

	return TextureInternalFormat::RGB;
}

void Texture::extractTextureDataFromSettings(const TextureLoadDescriptor& settings, TextureData& textureData)
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

	//TextureData textureData;

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

void Texture::extractTextureDataFromFile(const std::string& fileLocation, TextureData& textureData)
{
	// flip if needed
	STBIHelper::setFlip(textureData.flip);

	std::filesystem::path p(fileLocation);
	std::string ext = p.extension().string();

	if (ext == ".exr")
	{
		EXRLoader::loadSingleChannelEXR(fileLocation, textureData.width, textureData.height, textureData.data);
		textureData.type = TextureType::FLOAT;
		textureData.channels = 1;

		//InspectEXRChannels(fileLocation.c_str());
	}
	else if (STBIHelper::isHDR(fileLocation.c_str()))
	{
		textureData.data = STBIHelper::loadImageFloat(fileLocation, &textureData.width, &textureData.height, &textureData.channels);
		textureData.type = TextureType::FLOAT;

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
		textureData.type = TextureType::UNSIGNED_BYTE;
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

void TextureAsset::serialize(nlohmann::json& j) const
{
	// Texture assets currently do not have additional persistent data
	// beyond what is stored in the resource files and asset record.
	// Keep this as an empty object so the schema is explicit and can
	// be extended later without breaking existing data.
	j = nlohmann::json::object();
}

void TextureAsset::deserialize(const nlohmann::json& j)
{
	// No-op for now since TextureAsset has no custom serialized fields.
	// This is a placeholder to allow future extension of texture-specific
	// metadata without changing the persistence API.
	(void)j;
}
