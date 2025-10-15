#pragma once

#include <map>
#include "memory/ResourceWrapper.h"

class Texture;

class EngineAPI CommonTextures
{
public:
	static CommonTextures* create();
	static CommonTextures* load();
	void close();
	ResourceWrapper<Texture> getTexture(const std::string& name);

	CommonTextures(const CommonTextures&) = delete;
	CommonTextures& operator=(const CommonTextures&) = delete;
private:
	CommonTextures();
	void acquireTexture(const std::string& name, const std::string& path);

	std::map<std::string, ResourceWrapper<Texture>> m_textures;
};