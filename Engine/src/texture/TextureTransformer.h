#pragma once

#include "memory/AssetAliases.h"

class TextureTransformer
{
public:
	static TextureResourceRef flipVertical(TextureResourceRef texture);
	static void flipVertical(TextureResourceRef srcTexture, TextureResourceRef& dstTexture);

	static TextureResourceRef applyGammaCorrection(TextureResourceRef texture);
	static void applyGammaCorrection(TextureResourceRef srcTexture, TextureResourceRef& dstTexture);

	static TextureResourceRef texturePack(
		TextureResourceRef src0, int channel0,
		TextureResourceRef src1, int channel1,
		TextureResourceRef src2, int channel2,
		TextureResourceRef src3, int channel3);
};