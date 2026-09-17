#pragma once

#include "memory/AssetAliases.h"

class Animator;
class SSBO;

// GPU strategy: one compute-shader thread walks a whole skeleton for one instance
// (see SingleThreadPerInstanceBoneTransformComputeShader.glsl). Skeleton flattening, the
// per-skeleton SSBO cache, per-frame instance queueing and the dispatch itself all stay
// confined to this pair of files so BoneTransformSystem can swap in a different GPU
// approach later with no shared state to untangle.
class SingleThreadPerInstanceBoneTransform
{
public:
	static void beginFrame();
	static unsigned int addInstance(Animator& animator, const ModelResourceRef& model);
	static void endFrame(SSBO& animationBuffer);
};
