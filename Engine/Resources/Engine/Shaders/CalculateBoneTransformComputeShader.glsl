#compute

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
uniform float currentTime;

// ---- interpolation helpers (mirrors Bone::update on the CPU) ----

// Bone.cpp getProgress()
float getProgress(float t1, float t2, float v)
{
    return (v - t1) / (t2 - t1);
}

// Bone.cpp getDescreteIndex(): std::lower_bound(times, animationTime) - 1, clamped to [0, count - 2]
// so that p0Index + 1 always stays inside the channel.
int clampKeyIndex(uint lowerBound, uint count)
{
    return clamp(int(lowerBound) - 1, 0, int(count) - 2);
}

int getDescreteIndexPos(uint offset, uint count, float animationTime)
{
    uint lo = 0u, hi = count;
    while (lo < hi)
    {
        uint mid = (lo + hi) >> 1u;
        if (posKeys[offset + mid].w < animationTime) lo = mid + 1u; else hi = mid;
    }
    return clampKeyIndex(lo, count);
}

int getDescreteIndexScale(uint offset, uint count, float animationTime)
{
    uint lo = 0u, hi = count;
    while (lo < hi)
    {
        uint mid = (lo + hi) >> 1u;
        if (scaleKeys[offset + mid].w < animationTime) lo = mid + 1u; else hi = mid;
    }
    return clampKeyIndex(lo, count);
}

int getDescreteIndexRot(uint offset, uint count, float animationTime)
{
    uint lo = 0u, hi = count;
    while (lo < hi)
    {
        uint mid = (lo + hi) >> 1u;
        if (rotTimes[offset + mid] < animationTime) lo = mid + 1u; else hi = mid;
    }
    return clampKeyIndex(lo, count);
}

mat4 translateMat(vec3 t)
{
    mat4 m = mat4(1.0);
    m[3] = vec4(t, 1.0);
    return m;
}

mat4 scaleMat(vec3 s)
{
    mat4 m = mat4(1.0);
    m[0][0] = s.x;
    m[1][1] = s.y;
    m[2][2] = s.z;
    return m;
}

// quaternion packed as xyzw (w = real part), matching glm's default quat storage
mat4 quatToMat4(vec4 q)
{
    float qxx = q.x * q.x, qyy = q.y * q.y, qzz = q.z * q.z;
    float qxz = q.x * q.z, qxy = q.x * q.y, qyz = q.y * q.z;
    float qwx = q.w * q.x, qwy = q.w * q.y, qwz = q.w * q.z;

    mat4 m = mat4(1.0);
    m[0][0] = 1.0 - 2.0 * (qyy + qzz);
    m[0][1] =       2.0 * (qxy + qwz);
    m[0][2] =       2.0 * (qxz - qwy);

    m[1][0] =       2.0 * (qxy - qwz);
    m[1][1] = 1.0 - 2.0 * (qxx + qzz);
    m[1][2] =       2.0 * (qyz + qwx);

    m[2][0] =       2.0 * (qxz + qwy);
    m[2][1] =       2.0 * (qyz - qwx);
    m[2][2] = 1.0 - 2.0 * (qxx + qyy);
    return m;
}

// glm::slerp(): flips to the shorter arc, falls back to lerp when nearly parallel
vec4 quatSlerp(vec4 x, vec4 y, float a)
{
    vec4  z        = y;
    float cosTheta = dot(x, y);

    if (cosTheta < 0.0)
    {
        z        = -y;
        cosTheta = -cosTheta;
    }

    if (cosTheta > 1.0 - 1e-5)
    {
        return mix(x, z, a);
    }

    float angle = acos(cosTheta);
    return (sin((1.0 - a) * angle) * x + sin(a * angle) * z) / sin(angle);
}

// Bone::interpolatePosition()
mat4 interpolatePosition(BoneChannelInfo boneInfo, float animationTime)
{
    if (boneInfo.posCount == 1u)
    {
        return translateMat(posKeys[boneInfo.posOffset].xyz);
    }

    int p0Index = getDescreteIndexPos(boneInfo.posOffset, boneInfo.posCount, animationTime);
    int p1Index = p0Index + 1;
    vec4 k0 = posKeys[boneInfo.posOffset + uint(p0Index)];
    vec4 k1 = posKeys[boneInfo.posOffset + uint(p1Index)];
    float progress = getProgress(k0.w, k1.w, animationTime);
    return translateMat(mix(k0.xyz, k1.xyz, progress));
}

// Bone::interpolateRotation()
mat4 interpolateRotation(BoneChannelInfo boneInfo, float animationTime)
{
    if (boneInfo.rotCount == 1u)
    {
        return quatToMat4(normalize(rotKeys[boneInfo.rotOffset]));
    }

    int p0Index = getDescreteIndexRot(boneInfo.rotOffset, boneInfo.rotCount, animationTime);
    int p1Index = p0Index + 1;
    float t0 = rotTimes[boneInfo.rotOffset + uint(p0Index)];
    float t1 = rotTimes[boneInfo.rotOffset + uint(p1Index)];
    float progress = getProgress(t0, t1, animationTime);
    vec4 finalRotation = quatSlerp(rotKeys[boneInfo.rotOffset + uint(p0Index)],
                                   rotKeys[boneInfo.rotOffset + uint(p1Index)],
                                   progress);
    return quatToMat4(normalize(finalRotation));
}

// Bone::interpolateScale()
mat4 interpolateScale(BoneChannelInfo boneInfo, float animationTime)
{
    if (boneInfo.scaleCount == 1u)
    {
        return scaleMat(scaleKeys[boneInfo.scaleOffset].xyz);
    }

    int p0Index = getDescreteIndexScale(boneInfo.scaleOffset, boneInfo.scaleCount, animationTime);
    int p1Index = p0Index + 1;
    vec4 k0 = scaleKeys[boneInfo.scaleOffset + uint(p0Index)];
    vec4 k1 = scaleKeys[boneInfo.scaleOffset + uint(p1Index)];
    float progress = getProgress(k0.w, k1.w, animationTime);
    return scaleMat(mix(k0.xyz, k1.xyz, progress));
}

// Bone::update()
mat4 interpolateBone(BoneChannelInfo boneInfo, float animationTime)
{
    mat4 translation = interpolatePosition(boneInfo, animationTime);
    mat4 rotation    = interpolateRotation(boneInfo, animationTime);
    mat4 scale       = interpolateScale(boneInfo, animationTime);
    return translation * rotation * scale;
}

void main()
{
    if (gl_GlobalInvocationID.x >= totalBoneTransforms) 
    {
        return;
    }

    InstanceStateData state = instances[gl_GlobalInvocationID.x];
    NodeData nodeData = nodes[gl_GlobalInvocationID.x];

    mat4 nodeTransform = nodeData.transform;

    // If node is associated with a bone, set the local trasnform to the bone local transform
    if(nodeData.boneID != -1)
    {
        AnimClipInfo clipInfo = clips[state.clipID];
        BoneChannelInfo boneInfo = bones[clipInfo.boneTableOffset];

        mat4 boneTransform = interpolateBone(boneInfo, currentTime);
        nodeTransform = boneTransform;
    }

    // Go up the parent chain hierarchy
    int parentID = nodeData.parentID;
    while(parentID != -1)
    {
        NodeData parent = nodes[parentID];
        nodeTransform = parent.transform * nodeTransform;
        parentID = parent.parentID;
    }
    
    outputTransforms[gl_GlobalInvocationID.x] = nodeTransform;
}