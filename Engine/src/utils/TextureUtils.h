#pragma once

#include <cstdint>

#include "texture/Texture.h"

class TextureUtils
{
public:
	static uint32_t bytesPerChannel(TextureType type);

	static uint32_t channelCount(TextureFormat format);

	static ImageBuffer createBlankTextureBuffer2D(
		uint32_t width,
		uint32_t height,
		TextureFormat format,
		TextureType type);

	static ImageBuffer createBlankTextureBuffer3D(
		uint32_t width,
		uint32_t height,
		uint32_t depth,
		TextureFormat format,
		TextureType type);
};