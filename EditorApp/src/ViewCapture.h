#pragma once

#include <string>
#include <vector>
#include <cstdint>

// CPU copy of a render view, used by -capture and the AI screenshot tool
struct CapturedImage
{
	int width = 0;
	int height = 0;
	std::vector<uint8_t> rgb; // top row first, 3 bytes per pixel
};

// Reads a scene render view ("Editor View", "Game View") back from the GPU. Main thread only.
bool captureRenderView(const std::string& viewName, CapturedImage& out);

// Box-filters the image down by a whole factor so it is at most maxWidth wide
CapturedImage downscaleToWidth(const CapturedImage& image, int maxWidth);

std::vector<uint8_t> encodePng(const CapturedImage& image);
