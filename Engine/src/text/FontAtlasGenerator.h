#pragma once

#include <string>
#include <vector>
#include <cstdint>

#include <glm/glm.hpp>

struct FontAtlasGlyph
{
	uint32_t codepoint = 0;
	float advance = 0;
	glm::vec4 plane{}; // left, bottom, right, top in em, relative to the pen on the baseline
	glm::vec4 atlas{}; // left, bottom, right, top in atlas pixels
	bool whitespace = false;
};

struct FontAtlasData
{
	int width = 0;
	int height = 0;
	std::vector<uint8_t> pixels; // RGBA (MTSDF), bottom row first
	float emSize = 0;
	float distanceRange = 0;
	float lineHeight = 0;
	float ascender = 0;
	std::vector<FontAtlasGlyph> glyphs;
};

bool generateFontAtlas(const std::string& ttfPath, float emSize, float distanceRange, FontAtlasData& out);
