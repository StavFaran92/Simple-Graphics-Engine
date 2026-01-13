#include "TextureUtils.h"

uint32_t TextureUtils::bytesPerChannel(Texture::Type type)
{
	switch (type)
	{
	case Texture::Type::BYTE:
	case Texture::Type::UNSIGNED_BYTE:
		return 1;

	case Texture::Type::SHORT:
	case Texture::Type::UNSIGNED_SHORT:
		return 2;

	case Texture::Type::INT:
	case Texture::Type::UNSIGNED_INT:
	case Texture::Type::FLOAT:
		return 4;

	default:
		logError("Unsupported texture Type");
		return 0;
	}
}

uint32_t TextureUtils::channelCount(Texture::Format format)
{
	switch (format)
	{
	case Texture::Format::RED:
	case Texture::Format::GREEN:
	case Texture::Format::BLUE:
	case Texture::Format::ALPHA:
		return 1;

	case Texture::Format::RGB:
		return 3;

	case Texture::Format::RGBA:
		return 4;

	default:
		logError("Unsupported texture Format");
		return 0;
	}
}

std::vector<uint8_t> TextureUtils::createBlankTextureBuffer2D(uint32_t width, uint32_t height, Texture::Format format, Texture::Type type)
{
	const uint32_t bpp = channelCount(format) * bytesPerChannel(type);
	const size_t size = static_cast<size_t>(width) * height * bpp;
	return std::vector<uint8_t>(size, 0);
}

std::vector<uint8_t> TextureUtils::createBlankTextureBuffer3D(uint32_t width, uint32_t height, uint32_t depth, Texture::Format format, Texture::Type type)
{
	const uint32_t bpp = channelCount(format) * bytesPerChannel(type);
	const size_t size = static_cast<size_t>(width) * height * depth * bpp;
	return std::vector<uint8_t>(size, 0);
}