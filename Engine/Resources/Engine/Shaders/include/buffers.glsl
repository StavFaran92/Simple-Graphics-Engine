// ----- Buffers ----- //

struct InstanceData
{
	uint modelIndex;
	uint isAnimated;
	uint boneCount;
	uint _pad0; // explicit padding to 16 bytes to match InstanceData in Graphics.h
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
