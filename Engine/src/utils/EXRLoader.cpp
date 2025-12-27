#include "EXRLoader.h"

#include "core/logger.h"

#define TINYEXR_IMPLEMENTATION
#include "tinyexr.h"

bool EXRLoader::loadSingleChannelEXR(const std::string& filename, int& width, int& height, void*& outData)
{
	EXRVersion version;
	if (ParseEXRVersionFromFile(&version, filename.c_str()) != 0) {
		logError("Failed to parse EXR version.");
		return false;
	}

	EXRHeader header;
	InitEXRHeader(&header);
	const char* err = nullptr;

	if (ParseEXRHeaderFromFile(&header, &version, filename.c_str(), &err) != 0) {
		logError("Failed to parse EXR header. error: {}", err);
		FreeEXRErrorMessage(err);
		return false;
	}

	// Request float output
	header.requested_pixel_types = new int[header.num_channels];
	for (int i = 0; i < header.num_channels; i++) {
		header.requested_pixel_types[i] = TINYEXR_PIXELTYPE_FLOAT;
	}

	EXRImage image;
	InitEXRImage(&image);

	if (LoadEXRImageFromFile(&image, &header, filename.c_str(), &err) != TINYEXR_SUCCESS) {
		logError("Failed to load EXR image. error: {}", err);
		FreeEXRErrorMessage(err);
		FreeEXRHeader(&header);
		return false;
	}

	if (image.num_channels != 1) {
		logError("Expected 1-channel EXR, but got {}" , std::to_string(image.num_channels));
		FreeEXRImage(&image);
		FreeEXRHeader(&header);
		return false;
	}

	width = image.width;
	height = image.height;
	int pixelCount = width * height;

	float* buffer = new float[pixelCount];
	std::memcpy(buffer, image.images[0], pixelCount * sizeof(float));
	outData = static_cast<void*>(buffer);

	FreeEXRImage(&image);
	FreeEXRHeader(&header);
	return true;
}

bool EXRLoader::saveSingleChannelEXR(const std::string& filename, int width, int height, const void* data)
{
	EXRHeader header;
	InitEXRHeader(&header);

	EXRImage image;
	InitEXRImage(&image);

	image.num_channels = 1;

	std::vector<float> images[1];
	images[0].resize(width * height);

	const float* image_ptr[1];
	image_ptr[0] = &(((float*)data)[0]); // R

	image.images = (unsigned char**)image_ptr;
	image.width = width;
	image.height = height;

	header.num_channels = 1;
	header.channels = (EXRChannelInfo*)malloc(sizeof(EXRChannelInfo) * header.num_channels);
	// Must be (A)BGR order, since most of EXR viewers expect this channel order.
	strncpy(header.channels[0].name, "R", 255); 
	header.channels[0].name[strlen("R")] = '\0';

	header.pixel_types = (int*)malloc(sizeof(int) * header.num_channels);
	header.requested_pixel_types = (int*)malloc(sizeof(int) * header.num_channels);
	for (int i = 0; i < header.num_channels; i++) {
		header.pixel_types[i] = TINYEXR_PIXELTYPE_FLOAT; // pixel type of input image
		header.requested_pixel_types[i] = TINYEXR_PIXELTYPE_FLOAT; // pixel type of output image to be stored in .EXR
	}

	const char* err = NULL; // or nullptr in C++11 or later.
	int ret = SaveEXRImageToFile(&image, &header, filename.c_str(), &err);
	if (ret != TINYEXR_SUCCESS) {
		logError("Save EXR failed. error: {}", err);
		FreeEXRErrorMessage(err); // free's buffer for an error message
		return ret;
	}
	logInfo("Saved exr file. {}", filename);

	free(header.channels);
	free(header.pixel_types);
	free(header.requested_pixel_types);

	return true;
}

void EXRLoader::inspectEXR(const std::string& filename)
{
	EXRVersion exr_version;
	if (ParseEXRVersionFromFile(&exr_version, filename.c_str()) != 0) {
		logError("Failed to parse EXR version");
		return;
	}

	EXRHeader header;
	InitEXRHeader(&header);
	const char* err = nullptr;

	if (ParseEXRHeaderFromFile(&header, &exr_version, filename.c_str(), &err) != 0) {
		logError("Failed to parse EXR header. error: {} ", err);;
		FreeEXRErrorMessage(err);
		return;
	}

	logDebug("Channels in EXR file: ");
	for (int i = 0; i < header.num_channels; ++i) {
		std::string name = header.channels[i].name;
		std::string typeStr = "UNKNOWN";
		int type = header.pixel_types[i];
		if (type == TINYEXR_PIXELTYPE_HALF) typeStr = "half";
		else if (type == TINYEXR_PIXELTYPE_FLOAT) typeStr = "float";
		else if (type == TINYEXR_PIXELTYPE_UINT) typeStr = "uint";

		logDebug(" {} - ({})" , name , typeStr);
	}

	FreeEXRHeader(&header);
}
