#compute

#version 430 core

layout (local_size_x = 32) in;

// SSBO input struct
layout(std430, binding = 0) readonly buffer InstanceData {
    vec4 positions[]; // xyz = position, w = instanceID
};

// SSBO output struct
layout(std430, binding = 1) writeonly buffer VisibleBuffer {
    // uint visibleIDs[];
    vec4 outputPositions[];
};

// Frustum planes ordered
// - znear, 
// - zfar, 
// - right, 
// - left, 
// - up, 
// - down, 
layout(std140, binding = 2) uniform CameraFrustum {
    vec4 planes[6]; // Store plane.normal.xyz and plane.d
};

layout(binding = 0) uniform atomic_uint visibleCounter;
uniform int positionsSize;

bool isInFrustum(vec3 pos)
{
    return true;
}

void main() 
{
    if (gl_GlobalInvocationID.x >= positionsSize)
    {
        return;
    }

    vec4 posData = positions[gl_GlobalInvocationID.x];
    uint idx = floatBitsToUint(posData.w);
    vec3 pos = posData.xyz;
    if(isInFrustum(pos))
    {
        uint writeIndex = atomicCounterIncrement(visibleCounter);
        // visibleIDs[writeIndex] = idx;
        outputPositions[writeIndex] = posData;
    }
}