#include "animation/BoneTransformSystem.h"

#if SGE_BONE_TRANSFORM_STRATEGY == SGE_BONE_TRANSFORM_SINGLE_THREAD_PER_INSTANCE
	#include "animation/SingleThreadPerInstanceBoneTransform.h"
	using ActiveBoneTransformStrategy = SingleThreadPerInstanceBoneTransform;
#else
	#error "Unknown SGE_BONE_TRANSFORM_STRATEGY"
#endif

void BoneTransformSystem::beginFrame()
{
	ActiveBoneTransformStrategy::beginFrame();
}

unsigned int BoneTransformSystem::addInstance(Animator& animator, const ModelResourceRef& model)
{
	return ActiveBoneTransformStrategy::addInstance(animator, model);
}

void BoneTransformSystem::endFrame(SSBO& animationBuffer)
{
	ActiveBoneTransformStrategy::endFrame(animationBuffer);
}
