#pragma once

#include <string>

#include "core/Core.h"

struct TextureData;

// Binary serializer for TextureData (.texture files)
class EngineAPI TextureBinaryLoader
{
public:
	// Serialize a texture into a binary .texture file.
	// Returns true on success, false on failure.
	static bool save(const TextureData& texture, const std::string& targetFile);

	// Load texture from a binary .texture file.
	// Returns true on success, false on failure.
	static bool load(const std::string& sourceFile, TextureData& outTexture);
};
