#include "animation/AnimationSystem.h"

#include "core/Engine.h"
#include "runtime/Scene.h"
#include "animation/Animator.h"
#include "component/MeshRendererComponent.h"

namespace
{
	// Upper bounds on what a single dispatch can cover. Sized like the instancing
	// buffers in Graphics::Graphics().
	constexpr int maxAnimatedInstances = 5000;
	constexpr int maxNodesPerInstance = 100;   // matches MAX_BONES in animation.glsl
	constexpr int maxNodes = maxAnimatedInstances * maxNodesPerInstance;

	// Key data is per clip, not per instance - one pool shared by every loaded animation.
	constexpr int maxKeys = 1 << 16;
	constexpr int maxClips = 256;
	constexpr int maxBoneChannels = maxClips * maxNodesPerInstance;
}

AnimationSystem::AnimationSystem()
{
}

bool AnimationSystem::init()
{
	getBuffer(AnimationBuffer::Nodes).allocate(maxNodes * sizeof(AnimationNodeData));
	getBuffer(AnimationBuffer::PosKeys).allocate(maxKeys * sizeof(glm::vec4));
	getBuffer(AnimationBuffer::ScaleKeys).allocate(maxKeys * sizeof(glm::vec4));
	getBuffer(AnimationBuffer::RotKeys).allocate(maxKeys * sizeof(glm::vec4));
	getBuffer(AnimationBuffer::RotTimes).allocate(maxKeys * sizeof(float));
	getBuffer(AnimationBuffer::BoneTable).allocate(maxBoneChannels * sizeof(BoneChannelInfo));
	getBuffer(AnimationBuffer::Clips).allocate(maxClips * sizeof(AnimClipInfo));
	getBuffer(AnimationBuffer::InstanceState).allocate(maxNodes * sizeof(InstanceStateData));
	getBuffer(AnimationBuffer::FinalBoneMatrices).allocate(maxNodes * sizeof(glm::mat4));

	for (int i = 0; i < static_cast<int>(AnimationBuffer::Count); i++)
	{
		m_buffers[i].setSlot(i);
	}

	m_CalculateBoneTransformCS = Shader::load(SGE_ROOT_DIR "Resources/Engine/Shaders/CalculateBoneTransformComputeShader.glsl");

	return true;
}

void AnimationSystem::bindBuffers()
{
	for (auto& buffer : m_buffers)
	{
		buffer.bind();
	}
}

void AnimationSystem::update(Scene* scene, float dt)
{
	for (auto&& [e, animator, mesh] : scene->getRegistry().getRegistry().view<Animator, MeshRendererComponent>().each())
	{
		Entity entity(e, &scene->getRegistry());
		animator.update(entity, dt);
	}
}
