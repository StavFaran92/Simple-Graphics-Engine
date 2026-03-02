#pragma once

#include <vector>
#include <string>

#include "core/Core.h"

struct MeshData;

// Binary serializer for MeshData arrays (.mesh files)
class EngineAPI MeshBinaryLoader
{
public:
	// Serialize a list of meshes into a binary .mesh file.
	// Returns true on success, false on failure.
	static bool save(const std::vector<MeshData>& meshes, const std::string& targetFile);

	// Load meshes from a binary .mesh file.
	// Returns true on success, false on failure and clears outMeshes.
	static bool load(const std::string& sourceFile, std::vector<MeshData>& outMeshes);
};

