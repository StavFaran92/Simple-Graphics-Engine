#pragma once

#include "core/Core.h"
#include "memory/AssetAliases.h"

class Animator;
class SSBO;

// Picks which GPU strategy computes skeletal bone-hierarchy transforms for instanced rendering.
// Add a new strategy by dropping in its own confined pair of files (see SingleThreadPerInstanceBoneTransform
// for the shape to follow) and adding one more branch in the .cpp - no interface, just a compile-time redirect,
// so a handful of GPU approaches can be tried side by side without touching each other.
#define SGE_BONE_TRANSFORM_SINGLE_THREAD_PER_INSTANCE 1

#define SGE_BONE_TRANSFORM_STRATEGY SGE_BONE_TRANSFORM_SINGLE_THREAD_PER_INSTANCE

class EngineAPI BoneTransformSystem
{
public:
	// Call once per frame, right after the instanced animation SSBO's cursor is reset.
	static void beginFrame();

	// Reserves this entity's bone-matrix slot and schedules its GPU evaluation.
	// Returns the mat4-unit offset into the animation SSBO where the final matrices will land
	// (this is what InstanceData::modelIndex should be set to).
	static unsigned int addInstance(Animator& animator, const ModelResourceRef& model);

	// Call once per frame after all addInstance calls for this pass, before the instanced draw calls.
	// Dispatches the scheduled GPU work, writing results directly into animationBuffer.
	static void endFrame(SSBO& animationBuffer);
};
