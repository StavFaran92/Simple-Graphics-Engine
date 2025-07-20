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
layout(std140, binding = 0) uniform CameraFrustum {
    vec4 planes[6]; // Store plane.normal.xyz and plane.d
};

layout(binding = 0) uniform atomic_uint visibleCounter;
uniform int positionsSize;

float getSignedDistanceToPlane(vec3 pos, vec4 plane)
{
    return dot(plane.xyz, pos) - plane.w;
}

bool isForwardOfPlane(vec3 pos, vec4 plane)
{
    // Compute the projection interval radius of b onto L(t) = b.c + t * p.n
    float r = 10 * abs(plane.x) + 10 * abs(plane.y) + 10 * abs(plane.z);

    return -r <= getSignedDistanceToPlane(pos, plane);
}

bool isInFrustum(vec3 pos)
{
    return isForwardOfPlane(pos, planes[0]) &&
		isForwardOfPlane(pos, planes[1]) &&
		isForwardOfPlane(pos, planes[2]) &&
		isForwardOfPlane(pos, planes[3]) &&
		isForwardOfPlane(pos, planes[4]) &&
		isForwardOfPlane(pos, planes[5]);
}

void main() 
{
    if (gl_GlobalInvocationID.x >= positionsSize)
    {
        return;
    }

    vec4 posData = positions[gl_GlobalInvocationID.x];
    //uint idx = floatBitsToUint(posData.w);
    vec3 pos = posData.xyz;
    if(isInFrustum(pos))
    {
        uint writeIndex = atomicCounterIncrement(visibleCounter);
        // visibleIDs[writeIndex] = idx;
        outputPositions[writeIndex] = posData;
        outputPositions[writeIndex].w = gl_GlobalInvocationID.x;
    }
}