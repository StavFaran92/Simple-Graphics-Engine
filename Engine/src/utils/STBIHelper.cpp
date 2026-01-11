#include "STBIHelper.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

unsigned char* STBIHelper::decodeCompressedFromMemory(const unsigned char* rawBuffer, int len, int* outWidth, int* outHeight, int* outChannels)
{
	auto buffer = stbi_load_from_memory(rawBuffer, len, outWidth, outHeight, outChannels, 3);
	return buffer;
}

void STBIHelper::writeToPNG(const std::string& filename, int x, int y, int comp, const void* data, int stride_bytes)
{
	stbi_write_png(filename.c_str(), x, y, comp, data, stride_bytes);
}

void STBIHelper::setFlip(bool flip)
{
	stbi_set_flip_vertically_on_load(flip);
}

bool STBIHelper::isHDR(const std::string& filename)
{
	return stbi_is_hdr(filename.c_str()) != 0;
}

void* STBIHelper::loadImageFloat(const std::string& filename, int* x, int* y, int* comp)
{
	return stbi_loadf(filename.c_str(), x, y, comp, 0);
}

void* STBIHelper::loadImage(const std::string& filename, int* x, int* y, int* comp)
{
	return stbi_load(filename.c_str(), x, y, comp, 0);
}
