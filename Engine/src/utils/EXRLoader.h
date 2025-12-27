#pragma once

#include <string>

class EXRLoader
{
public:
	static bool loadSingleChannelEXR(const std::string& filename, int& width, int& height, void*& outData);

	static bool saveSingleChannelEXR(const std::string& filename, int width, int height, const void* data);

	static void inspectEXR(const std::string& filename);
};