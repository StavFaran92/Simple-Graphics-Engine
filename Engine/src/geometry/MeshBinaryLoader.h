#pragma once

#include <vector>
#include <string>

#include "core/Core.h"

struct ModelData;

// Binary serializer for ModelData (.mesh files)
class EngineAPI ModelBinaryLoader
{
public:
	// Serialize model data into a binary .mesh file.
	// Returns true on success, false on failure.
	static bool save(const ModelData& modelData, const std::string& targetFile);

	// Load model data from a binary .mesh file.
	// Returns true on success, false on failure and clears outModelData.
	static bool load(const std::string& sourceFile, ModelData& outModelData);
};

