#pragma once

#include <string>

class STBIHelper
{
public:
	static unsigned char* decodeCompressedFromMemory(const unsigned char* rawBuffer, int len, int* outWidth, int* outHeight, int* outChannels);

	static void writeToPNG(const std::string& filename, int x, int y, int comp, const void* data, int stride_bytes);

	static void setFlip(bool);

	static bool isHDR(const std::string& filename);

	static std::vector<uint8_t>loadImageFloat(const std::string& filename, int* x, int* y, int* comp);

	static std::vector<uint8_t> loadImage(const std::string& filename, int* x, int* y, int* comp);
};