#pragma once

template<typename T> class ResourceWrapper;
class Texture;

class TextureTransformer
{
public:
	static ResourceWrapper<Texture> flipVertical(ResourceWrapper<Texture> texture);
	static void flipVertical(ResourceWrapper<Texture> srcTexture, ResourceWrapper<Texture>& dstTexture);

	static ResourceWrapper<Texture> applyGammaCorrection(ResourceWrapper<Texture> texture);
	static void applyGammaCorrection(ResourceWrapper<Texture> srcTexture, ResourceWrapper<Texture>& dstTexture);
};