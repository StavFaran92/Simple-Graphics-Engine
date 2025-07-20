#compute

#version 460 core

layout (local_size_x = 32, local_size_y = 32) in;

// SSBO input struct
layout(std430, binding = 0) readonly buffer InstanceData {
    vec4 positions[]; // xyz = position, w = instanceID
};

// SSBO output struct
layout(std430, binding = 1) writeonly buffer VisibleBuffer {
    vec4 outputPositions[];
};

layout(std140, binding = 0) uniform RandomPatchSample { // todo give better name
    vec4 randomPatchSample[255]; 
};

layout(binding = 0) uniform atomic_uint visibleCounter;
uniform int textureWidth;
uniform int textureHeight;
layout(rgba8, binding = 0) uniform image2D spreadMap;

void main() 
{
    if (gl_GlobalInvocationID.x >= textureWidth || gl_GlobalInvocationID.y >= textureHeight) 
    {
        return;
    }

    vec4 pos = positions[gl_GlobalInvocationID.y * textureWidth + gl_GlobalInvocationID.x];
    int idY = int(pos.w) / textureWidth;
    int idX = int(pos.w) % textureWidth;

    uint spread = uint(floor(imageLoad(spreadMap, ivec2(idX, idY)).r * 255.0));
    if(spread == 0)
        return;
    uint writeIndex = atomicCounterAdd(visibleCounter, spread);
    for(int i=0; i<spread; i++)
    {
        outputPositions[writeIndex] = pos + randomPatchSample[i];
        outputPositions[writeIndex].w = 1.; // todo revert, this is needed
        // outputPositions[writeIndex].w = gl_GlobalInvocationID.y * textureWidth + gl_GlobalInvocationID.x;
        writeIndex++;
    }
}