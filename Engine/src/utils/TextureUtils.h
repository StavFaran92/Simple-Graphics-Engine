#pragma once

#include <cstdint>

#include "texture/Texture.h"

class TextureUtils
{
public:
	static uint32_t bytesPerChannel(Texture::Type type);

	static uint32_t channelCount(Texture::Format format);

	static std::vector<uint8_t> createBlankTextureBuffer2D(
		uint32_t width,
		uint32_t height,
		Texture::Format format,
		Texture::Type type);

	static std::vector<uint8_t> createBlankTextureBuffer3D(
		uint32_t width,
		uint32_t height,
		uint32_t depth,
		Texture::Format format,
		Texture::Type type);
};