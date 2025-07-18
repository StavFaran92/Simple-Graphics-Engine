#compute

#version 430 core

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
uniform int positionsSize;
uniform sampler2D spreadMap;

void main() 
{
    if (gl_GlobalInvocationID.x >= positionsSize) // todo fix
    {
        return;
    }
    
    int spread = floatBitsToint(texture(spreadMap, gl_GlobalInvocationID.xy).r * 255);
    uint writeIndex = atomicAdd(visibleCounter, spread);
    vec4 originalPosData = positions[gl_GlobalInvocationID.x]; // get correct (should depend also on Y)
    for(int i=0; i<spread; i++)
    {
        outputPositions[writeIndex] = originalPosData + randomPatchSample[i];
        writeIndex++;
    }
}