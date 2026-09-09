#version 430 core

layout (local_size_x = 32) in;

struct NodeData
{
    mat4 transform;
    int  parentID;    // -1 for root
    uint id;
}

// SSBO input struct
layout(std430, binding = 0) readonly buffer AnimationPoseData {
    NodeData animationPoseData[];
};

// packed: xyz = value, w = time
layout(std430, binding = 1) readonly buffer PosKeys   { vec4 posKeys[];   };   // xyz pos,   w time
layout(std430, binding = 2) readonly buffer ScaleKeys { vec4 scaleKeys[]; };   // xyz scale, w time

// rotation: quat needs all 4 lanes, so time goes in a parallel array
layout(std430, binding = 3) readonly buffer RotKeys     { vec4  rotKeys[];  };  // xyzw quat
layout(std430, binding = 4) readonly buffer RotTimes    { float rotTimes[]; };  // parallel time array

layout(std430, binding = 5) readonly buffer BoneTable {
    uint posOffset, posCount;
    uint rotOffset, rotCount;
    uint scaleOffset, scaleCount;
} boneTable[];