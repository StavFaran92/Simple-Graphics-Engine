#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <map>

#include "core/Core.h"
#include "core/ApplicationConstants.h"
#include "Configurations.h"
#include "Resource.h"
#include "Texture.h"

class EngineAPI Cubemap
{
public:
	static Resource<Texture> createCubemapFromCubemapFiles(const std::vector<std::string>& fileLocations);
	//static Resource<Texture> createCubemapFromEquirectangularFile(const std::string& fileLocation);

	static Resource<Texture> createCubemapFromBuffer(const Texture::TextureData& cubemapData);

	static Resource<Texture> createDefaultCubemap();
	static Resource<Texture> createEmptyCubemap(int width, int height, int internalFormat, int format, int type);
	static Resource<Texture> createEmptyCubemap(int width, int height, int internalFormat, int format, int type, std::map<int, int> params, bool createMipMaps = false);

	static Texture::TextureData extractCubemapDataFromEquirectangularFile(const std::string& fileLocation);
	static Texture::TextureData extractCubemapDataFromCubemapFiles(const std::vector<std::string>& files);
	static void saveEquirectangularMap(Resource<Texture> equirectangularMap);

	static Resource<Texture> build(const Texture::TextureData& textureData);
};
