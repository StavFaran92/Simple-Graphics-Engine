#include "STBIHelper.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

unsigned char* STBIHelper::decodeCompressedFromMemory(const unsigned char* rawBuffer, int len, int* outWidth, int* outHeight, int* outChannels)
{
	auto buffer = stbi_load_from_memory(rawBuffer, len, outWidth, outHeight, outChannels, 0);
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

std::vector<uint8_t> STBIHelper::loadImageFloat(
	const std::string& filename,
	int* x, int* y, int* comp)
{
	float* data = stbi_loadf(filename.c_str(), x, y, comp, 0);
	if (!data)
		return {};

	size_t count = static_cast<size_t>(*x)
		* static_cast<size_t>(*y)
		* static_cast<size_t>(*comp);

	std::vector<uint8_t> buffer(count * sizeof(float));
	std::memcpy(buffer.data(), data, buffer.size());

	stbi_image_free(data);
	return buffer;
}

std::vector<uint8_t> STBIHelper::loadImage(
	const std::string& filename,
	int* x, int* y, int* comp)
{
	unsigned char* data = stbi_load(filename.c_str(), x, y, comp, 0);
	if (!data)
		return {};

	size_t count = static_cast<size_t>(*x)
		* static_cast<size_t>(*y)
		* static_cast<size_t>(*comp);

	std::vector<uint8_t> buffer(count);
	std::memcpy(buffer.data(), data, count);

	stbi_image_free(data);
	return buffer;
}