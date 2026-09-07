// ----- Buffers ----- //

uniform bool isGpuInstanced;

struct InstanceData
{
	uint modelIndex;
	uint isAnimated;
	uint boneCount;
};

layout(std430, binding = 0) readonly buffer TransformBuffer {
    mat4 transformBuffer[];
};

layout(std430, binding = 1) readonly buffer AnimationBuffer {
    mat4 animationBuffer[];
};

layout(std430, binding = 2) readonly buffer InstanceDataBuffer {
    InstanceData instanceDataBuffer[];
};
