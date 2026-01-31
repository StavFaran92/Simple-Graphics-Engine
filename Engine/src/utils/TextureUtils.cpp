#include "TextureUtils.h"

#include "core/Logger.h"

uint32_t TextureUtils::bytesPerChannel(TextureType type)
{
	switch (type)
	{
	case TextureType::BYTE:
	case TextureType::UNSIGNED_BYTE:
		return 1;

	case TextureType::SHORT:
	case TextureType::UNSIGNED_SHORT:
		return 2;

	case TextureType::INT:
	case TextureType::UNSIGNED_INT:
	case TextureType::FLOAT:
		return 4;

	default:
		logError("Unsupported texture Type");
		return 0;
	}
}

uint32_t TextureUtils::channelCount(TextureFormat format)
{
	switch (format)
	{
	case TextureFormat::RED:
	case TextureFormat::GREEN:
	case TextureFormat::BLUE:
	case TextureFormat::ALPHA:
	case TextureFormat::DEPTH_COMPONENT:
		return 1;

	case TextureFormat::RG:
		return 2;

	case TextureFormat::RGB:
	case TextureFormat::RGB_INTEGER:
		return 3;

	case TextureFormat::RGBA:
		return 4;

	default:
		logError("Unsupported texture Format");
		return 0;
	}
}

void* TextureUtils::createBlankTextureBuffer2D(uint32_t width,
	uint32_t height,
	TextureFormat format,
	TextureType type)
{
	const uint32_t bpp = channelCount(format) * bytesPerChannel(type);
	const size_t size = static_cast<size_t>(width) * height * bpp;

	uint8_t* buffer = new uint8_t[size](); // zero-initialized
	return buffer;
}

void* TextureUtils::createBlankTextureBuffer3D(uint32_t width,
	uint32_t height,
	uint32_t depth,
	TextureFormat format,
	TextureType type)
{
	const uint32_t bpp = channelCount(format) * bytesPerChannel(type);
	const size_t size = static_cast<size_t>(width) * height * depth * bpp;

	uint8_t* buffer = new uint8_t[size](); // zero-initialized
	return static_cast<void*>(buffer);
}