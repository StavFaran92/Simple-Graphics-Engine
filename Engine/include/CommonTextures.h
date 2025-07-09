#pragma once

#include <map>
#include "Resource.h"

class Texture;

class EngineAPI CommonTextures
{
public:
	enum class TextureType
	{
		BLACK_1X1,
		WHITE_1X1,
		CUBEMAP_WHITE_1X1
	};



	

	static CommonTextures* create();
	static CommonTextures* load();
	void close();
	Resource<Texture> getTexture(TextureType texType);

	CommonTextures(const CommonTextures&) = delete;
	CommonTextures& operator=(const CommonTextures&) = delete;
private:
	CommonTextures();

	std::map<TextureType, Resource<Texture>> m_textures;
};