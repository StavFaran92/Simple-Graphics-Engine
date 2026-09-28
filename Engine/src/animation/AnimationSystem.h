#pragma once

#include <array>

#include "glm/glm.hpp"
#include "render/SSBO.h"
#include "systems/SubSystem.h"
#include "memory/AssetAliases.h"

class Scene;

// ---- CPU mirrors of the structs in CalculateBoneTransformComputeShader.glsl ----
// Keep these std430 friendly: uint/float only, and pad so the sizes match what the
// shader computes for each element.

// AnimationBuffer::Nodes
struct AnimationNodeData
{
	glm::mat4 transform;
	int parentID = -1;          // -1 for root
	int boneID = -1;            // -1 means no related bone
	int _pad[2] = { 0, 0 };     // std430 rounds the struct up to a vec4 boundary (80 bytes)
};
static_assert(sizeof(AnimationNodeData) == 80, "AnimationNodeData must match NodeData in the compute shader");

// Structs below are padded explicitly to a 16-byte multiple on both sides, since drivers
// disagree on whether std430 rounds small structs up to a vec4 boundary.

// AnimationBuffer::BoneTable
struct BoneChannelInfo
{
	unsigned int posOffset = 0, posCount = 0;
	unsigned int rotOffset = 0, rotCount = 0;
	unsigned int scaleOffset = 0, scaleCount = 0;
	unsigned int _pad[2] = { 0, 0 };
};
static_assert(sizeof(BoneChannelInfo) == 32, "BoneChannelInfo must match the compute shader's std430 layout");

// AnimationBuffer::Clips
struct AnimClipInfo
{
	unsigned int boneTableOffset = 0;   // where this clip's bones begin in bones[]
	unsigned int boneCount = 0;         // bones in this clip
	unsigned int _pad[2] = { 0, 0 };
};
static_assert(sizeof(AnimClipInfo) == 16, "AnimClipInfo must match the compute shader's std430 layout");

// AnimationBuffer::InstanceState
struct InstanceStateData
{
	unsigned int clipID = 0;
	float time = 0.f;                   // current sample time for this instance
	float _pad[2] = { 0.f, 0.f };
};
static_assert(sizeof(InstanceStateData) == 16, "InstanceStateData must match the compute shader's std430 layout");

// Doubles as the SSBO binding point - the values must match the layout(binding = N)
// declarations in CalculateBoneTransformComputeShader.glsl.
enum class AnimationBuffer
{
	Nodes = 0,          // NodeData nodes[]
	PosKeys = 1,        // vec4 posKeys[]   (xyz pos,   w time)
	ScaleKeys = 2,      // vec4 scaleKeys[] (xyz scale, w time)
	RotKeys = 3,        // vec4 rotKeys[]   (xyzw quat)
	RotTimes = 4,       // float rotTimes[] (parallel to rotKeys)
	BoneTable = 5,      // BoneChannelInfo bones[]
	Clips = 6,          // AnimClipInfo clips[]
	InstanceState = 7,  // InstanceStateData instances[]
	FinalBoneMatrices = 8, // mat4 outputTransforms[]

	Count
};

class EngineAPI AnimationSystem : public SubSystem
{
public:
	AnimationSystem();

	// Allocates the compute shader's storage. Needs a live GL context, so it is called
	// from Engine::initSystems() rather than the constructor.
	bool init();

	void update(Scene* scene, float dt);

	// Binds every buffer to its binding point, ready for a dispatch.
	void bindBuffers();

	SSBO& getBuffer(AnimationBuffer buffer) { return m_buffers[static_cast<int>(buffer)]; }

private:
	std::array<SSBO, static_cast<int>(AnimationBuffer::Count)> m_buffers;

	ShaderResourceRef m_CalculateBoneTransformCS;
};
