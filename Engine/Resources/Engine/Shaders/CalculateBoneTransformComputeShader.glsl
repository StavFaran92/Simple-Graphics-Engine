#version 430 core

layout (local_size_x = 96) in;

struct NodeData {
    mat4 transform;
    int  parentID;    // -1 for root
    int  boneID;           // -1 means no related bone
};

struct BoneChannelInfo {
    uint posOffset,   posCount;
    uint rotOffset,   rotCount;
    uint scaleOffset, scaleCount;
};

struct AnimClipInfo {
    uint boneTableOffset;   // where this clip's bones begin in bones[]
    uint boneCount;         // bones in this clip
};

struct InstanceStateData {
    uint  clipID;
    float time;             // current sample time for this instance
};

// ---- buffers ----
layout(std430, binding = 0) readonly buffer AnimationPoseData {
    NodeData nodes[];
};

// packed: xyz = value, w = time
layout(std430, binding = 1) readonly buffer PosKeys   { vec4 posKeys[];   };   // xyz pos,   w time
layout(std430, binding = 2) readonly buffer ScaleKeys { vec4 scaleKeys[]; };   // xyz scale, w time

// rotation: quat needs all 4 lanes, so time goes in a parallel array
layout(std430, binding = 3) readonly buffer RotKeys   { vec4  rotKeys[];  };   // xyzw quat
layout(std430, binding = 4) readonly buffer RotTimes  { float rotTimes[]; };   // parallel time array

layout(std430, binding = 5) readonly buffer BoneTable {
    BoneChannelInfo bones[];
};

layout(std430, binding = 6) readonly buffer AnimationMeta {
    AnimClipInfo clips[];
};

layout(std430, binding = 7) readonly buffer InstanceState {
    InstanceStateData instances[];
};

// SSBO output struct
layout(std430, binding = 8) writeonly buffer FinalBoneMatrices {
    mat4 outputTransforms[];
};


uniform uint totalBoneTransforms;

void main() 
{
    if (gl_GlobalInvocationID.x >= totalBoneTransforms) 
    {
        return;
    }

    InstanceState state = instances[gl_GlobalInvocationID.x];
    NodeData nodeData = nodes[gl_GlobalInvocationID.x];

    mat4 nodeTransform(1.0);
    if(nodeData.boneID != -1)
    {
        AnimClipInfo clipInfo = clips[state.clipID];
        BoneChannelInfo boneInfo = bones[clipInfo.boneTableOffset];

        //interpolate

        // node transform = bone transform
    }

    if(nodeData.parentID != -1)
    {
        nodeTransform = nodes[nodeData.parentID].transform * nodeTransform;
    }
    
    outputTransforms[gl_GlobalInvocationID.x] = nodeTransform;
}