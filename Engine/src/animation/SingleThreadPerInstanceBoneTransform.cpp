#include "animation/SingleThreadPerInstanceBoneTransform.h"

#include <cstdint>
#include <map>
#include <unordered_map>
#include <utility>
#include <vector>

#include <gl/glew.h>

#include "animation/Animation.h"
#include "animation/AnimationEntry.h"
#include "animation/Animator.h"
#include "animation/Bone.h"
#include "core/Configurations.h"
#include "geometry/Model.h"
#include "memory/BuiltInResources.h"
#include "render/SSBO.h"
#include "render/Shader.h"

namespace
{
	// Binding points used only while this strategy's compute shader is dispatched.
	// Slots 0-2 are reserved by the render loop (transform/animation/instance-data buffers,
	// see include/buffers.glsl) - keep clear of those.
	constexpr int SLOT_ANIMATION_OUTPUT = 1;
	constexpr int SLOT_NODE_DATA = 3;
	constexpr int SLOT_POS_KEYS = 4;
	constexpr int SLOT_SCALE_KEYS = 5;
	constexpr int SLOT_ROT_KEYS = 6;
	constexpr int SLOT_ROT_TIMES = 7;
	constexpr int SLOT_BONE_OFFSETS = 8;
	constexpr int SLOT_INSTANCE_PARAMS = 9;

	// Mirrors NodeData in SingleThreadPerInstanceBoneTransformComputeShader.glsl - keep std430 friendly.
	struct GpuNodeData
	{
		glm::mat4 localBindTransform{ 1.0f };
		int32_t parentID = -1; // -1 for the root node
		int32_t boneID = -1;   // -1 if this node has no skinning influence
		uint32_t posOffset = 0;
		uint32_t posCount = 0;
		uint32_t rotOffset = 0;
		uint32_t rotCount = 0;
		uint32_t scaleOffset = 0;
		uint32_t scaleCount = 0;
	};
	static_assert(sizeof(GpuNodeData) == 96, "GpuNodeData must match the compute shader's std430 layout");

	// Mirrors InstanceParams in the compute shader.
	struct GpuInstanceParams
	{
		uint32_t outputBoneOffset = 0; // mat4-unit offset into the animation output buffer
		float currentTime = 0.0f;
		float _pad0 = 0.0f;
		float _pad1 = 0.0f;
	};
	static_assert(sizeof(GpuInstanceParams) == 16, "GpuInstanceParams must match the compute shader's std430 layout");

	// Everything the compute shader needs to evaluate one (Animation, Model) skeleton pairing.
	// Built once and cached - the skeleton hierarchy, keyframes and bind pose never change frame to frame.
	struct GpuSkeletonBinding
	{
		AnimationResourceRef animation;
		ModelResourceRef model;
		unsigned int nodeCount = 0;
		SSBO nodeDataBuffer;
		SSBO posKeysBuffer;
		SSBO scaleKeysBuffer;
		SSBO rotKeysBuffer;
		SSBO rotTimesBuffer;
		SSBO boneOffsetsBuffer;
	};

	struct QueuedInstance
	{
		GpuSkeletonBinding* binding;
		GpuInstanceParams params;
	};

	std::map<std::pair<ResourceID, ResourceID>, GpuSkeletonBinding> g_bindings;
	std::vector<QueuedInstance> g_queuedInstances;
	unsigned int g_outputCursorMat4 = 0;

	template<typename T>
	void uploadBuffer(SSBO& buffer, int slot, const std::vector<T>& data)
	{
		size_t elementCount = data.empty() ? 1 : data.size();
		buffer.allocate(static_cast<int>(sizeof(T) * elementCount));
		buffer.setSlot(slot);
		buffer.bind();

		if (!data.empty())
		{
			buffer.setData(static_cast<int>(sizeof(T) * data.size()), data.data());
		}
	}

	GpuSkeletonBinding& getOrBuildBinding(const AnimationResourceRef& animation, const ModelResourceRef& model)
	{
		auto key = std::make_pair(animation.getUID(), model.getUID());

		// try_emplace constructs the binding (and its SSBO members) in place inside the map.
		// Copying or moving a GpuSkeletonBinding afterwards would duplicate its GL buffer ids
		// and double-free them once one of the copies is destroyed, so it must never leave the map.
		auto [it, inserted] = g_bindings.try_emplace(key);
		GpuSkeletonBinding& binding = it->second;
		if (!inserted)
		{
			return binding;
		}

		binding.animation = animation;
		binding.model = model;

		const AnimationData& data = animation->getData();
		binding.nodeCount = static_cast<unsigned int>(data.nodes.size());

		// The loader (AnimationLoader::readSceneNodeData) only stores child indices, and always
		// appends a node before recursing into its children, so a reverse pass here is enough to
		// recover a parent index per node while keeping parent-before-child ordering intact -
		// exactly the property the compute shader's serial hierarchy walk depends on.
		std::vector<int> parentID(data.nodes.size(), -1);
		for (size_t i = 0; i < data.nodes.size(); ++i)
		{
			for (int child : data.nodes[i].children)
			{
				parentID[child] = static_cast<int>(i);
			}
		}

		std::vector<GpuNodeData> nodes(data.nodes.size());
		std::vector<glm::vec4> posKeys;
		std::vector<glm::vec4> scaleKeys;
		std::vector<glm::vec4> rotKeys;
		std::vector<float> rotTimes;

		for (size_t i = 0; i < data.nodes.size(); ++i)
		{
			const MeshNodeData& node = data.nodes[i];

			GpuNodeData gpuNode;
			gpuNode.localBindTransform = node.transformation;
			gpuNode.parentID = parentID[i];
			gpuNode.boneID = model->getBoneID(node.name);

			auto boneIt = data.bones.find(node.name);
			if (boneIt != data.bones.end())
			{
				const auto& bone = boneIt->second;

				gpuNode.posOffset = static_cast<unsigned int>(posKeys.size());
				for (const auto& key : bone->getPositions())
				{
					posKeys.emplace_back(key.position, key.timeStamp);
				}
				gpuNode.posCount = static_cast<unsigned int>(posKeys.size() - gpuNode.posOffset);

				gpuNode.scaleOffset = static_cast<unsigned int>(scaleKeys.size());
				for (const auto& key : bone->getScales())
				{
					scaleKeys.emplace_back(key.scale, key.timeStamp);
				}
				gpuNode.scaleCount = static_cast<unsigned int>(scaleKeys.size() - gpuNode.scaleOffset);

				gpuNode.rotOffset = static_cast<unsigned int>(rotKeys.size());
				for (const auto& key : bone->getRotations())
				{
					rotKeys.emplace_back(key.orientation.x, key.orientation.y, key.orientation.z, key.orientation.w);
					rotTimes.push_back(key.timeStamp);
				}
				gpuNode.rotCount = static_cast<unsigned int>(rotKeys.size() - gpuNode.rotOffset);
			}

			nodes[i] = gpuNode;
		}

		uploadBuffer(binding.nodeDataBuffer, SLOT_NODE_DATA, nodes);
		uploadBuffer(binding.posKeysBuffer, SLOT_POS_KEYS, posKeys);
		uploadBuffer(binding.scaleKeysBuffer, SLOT_SCALE_KEYS, scaleKeys);
		uploadBuffer(binding.rotKeysBuffer, SLOT_ROT_KEYS, rotKeys);
		uploadBuffer(binding.rotTimesBuffer, SLOT_ROT_TIMES, rotTimes);
		uploadBuffer(binding.boneOffsetsBuffer, SLOT_BONE_OFFSETS, model->getBoneOffsets());

		return binding;
	}
}

void SingleThreadPerInstanceBoneTransform::beginFrame()
{
	g_queuedInstances.clear();
	g_outputCursorMat4 = 0;
}

unsigned int SingleThreadPerInstanceBoneTransform::addInstance(Animator& animator, const ModelResourceRef& model)
{
	const AnimationEntry* currentAnimation = animator.getCurrentAnimation();
	AnimationResourceRef animation = currentAnimation->animation.resource();

	GpuSkeletonBinding& binding = getOrBuildBinding(animation, model);

	unsigned int outputBoneOffset = g_outputCursorMat4;
	g_outputCursorMat4 += static_cast<unsigned int>(model->getBoneOffsets().size());

	GpuInstanceParams params;
	params.outputBoneOffset = outputBoneOffset;
	params.currentTime = animator.m_currentTime;

	g_queuedInstances.push_back({ &binding, params });

	return outputBoneOffset;
}

void SingleThreadPerInstanceBoneTransform::endFrame(SSBO& animationBuffer)
{
	if (g_queuedInstances.empty())
	{
		return;
	}

	ShaderResourceRef computeShader = BuiltInResources::get<Shader>(SGE_RESOURCE_SHADER_BONE_TRANSFORM_SINGLE_THREAD_PER_INSTANCE_COMPUTE);
	computeShader->use();

	animationBuffer.setSlot(SLOT_ANIMATION_OUTPUT);
	animationBuffer.bind();

	// A dispatch's node count and keyframe buffers apply to every thread in it, so instances must be
	// grouped by skeleton binding before dispatching - mixing skeletons in one dispatch would have some
	// instances walk the wrong hierarchy.
	std::unordered_map<GpuSkeletonBinding*, std::vector<GpuInstanceParams>> groups;
	for (const auto& queued : g_queuedInstances)
	{
		groups[queued.binding].push_back(queued.params);
	}

	static SSBO instanceParamsBuffer;

	for (auto& [binding, params] : groups)
	{
		binding->nodeDataBuffer.setSlot(SLOT_NODE_DATA);
		binding->nodeDataBuffer.bind();
		binding->posKeysBuffer.setSlot(SLOT_POS_KEYS);
		binding->posKeysBuffer.bind();
		binding->scaleKeysBuffer.setSlot(SLOT_SCALE_KEYS);
		binding->scaleKeysBuffer.bind();
		binding->rotKeysBuffer.setSlot(SLOT_ROT_KEYS);
		binding->rotKeysBuffer.bind();
		binding->rotTimesBuffer.setSlot(SLOT_ROT_TIMES);
		binding->rotTimesBuffer.bind();
		binding->boneOffsetsBuffer.setSlot(SLOT_BONE_OFFSETS);
		binding->boneOffsetsBuffer.bind();

		instanceParamsBuffer.allocate(static_cast<int>(sizeof(GpuInstanceParams) * params.size()));
		instanceParamsBuffer.setSlot(SLOT_INSTANCE_PARAMS);
		instanceParamsBuffer.bind();
		instanceParamsBuffer.setData(static_cast<int>(sizeof(GpuInstanceParams) * params.size()), params.data());

		computeShader->setUniformValue("nodeCount", binding->nodeCount);
		computeShader->setUniformValue("instanceCount", static_cast<unsigned int>(params.size()));

		GLuint groupCount = (static_cast<GLuint>(params.size()) + 7u) / 8u;
		glDispatchCompute(groupCount, 1, 1);
	}

	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}
