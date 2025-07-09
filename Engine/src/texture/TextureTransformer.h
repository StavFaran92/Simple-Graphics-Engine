#pragma once

template<typename T> class Resource;
class Texture;

class TextureTransformer
{
public:
	static Resource<Texture> flipVertical(Resource<Texture> texture);
	static void flipVertical(Resource<Texture> srcTexture, Resource<Texture>& dstTexture);
};