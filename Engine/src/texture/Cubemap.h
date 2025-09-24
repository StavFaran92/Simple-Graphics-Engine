#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <map>

#include "core/Core.h"
#include "core/ApplicationConstants.h"
#include "core/Configurations.h"
#include "memory/ResourceWrapper.h"
#include "texture/Texture.h"

class EngineAPI Cubemap
{
public:
	static ResourceWrapper<Texture> createCubemapFromCubemapFiles(const std::vector<std::string>& fileLocations);
	//static Resource<Texture> createCubemapFromEquirectangularFile(const std::string& fileLocation);

	static ResourceWrapper<Texture> createCubemapFromBuffer(const Texture::TextureData& cubemapData);

	static ResourceWrapper<Texture> createDefaultCubemap();
	static ResourceWrapper<Texture> createEmptyCubemap(int width, int height, int internalFormat, int format, int type);
	static ResourceWrapper<Texture> createEmptyCubemap(int width, int height, int internalFormat, int format, int type, std::map<int, int> params, bool createMipMaps = false);

	static Texture::TextureData extractCubemapDataFromEquirectangularFile(const std::string& fileLocation);
	static Texture::TextureData extractCubemapDataFromCubemapFiles(const std::vector<std::string>& files);
	static void saveEquirectangularMap(ResourceWrapper<Texture> equirectangularMap);

	static ResourceWrapper<Texture> build(const Texture::TextureData& textureData);
};
