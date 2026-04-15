#pragma once

#include "memory/AssetAliases.h"

class TextureTransformer
{
public:
	static TextureResourceRef flipVertical(TextureResourceRef texture);
	static void flipVertical(TextureResourceRef srcTexture, TextureResourceRef& dstTexture);

	static TextureResourceRef applyGammaCorrection(TextureResourceRef texture);
	static void applyGammaCorrection(TextureResourceRef srcTexture, TextureResourceRef& dstTexture);
};