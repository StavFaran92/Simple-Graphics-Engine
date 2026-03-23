#pragma once

#include <vector>
#include <string>

#include "core/Core.h"

struct AnimationData;

class AnimationBinaryLoader
{
public:
	// Serialize a animation into a binary .anim file.
	// Returns true on success, false on failure.
	static bool save(const AnimationData& animData, const std::string& targetFile);

	// Load animation from a binary .anim file.
	// Returns true on success, false on failure and clears outMeshes.
	static bool load(const std::string& sourceFile, AnimationData& outAnimData);
};