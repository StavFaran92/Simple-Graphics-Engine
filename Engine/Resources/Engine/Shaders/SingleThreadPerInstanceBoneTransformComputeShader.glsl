#compute

#version 430 core

// One thread walks one instance's entire skeleton hierarchy serially, sampling keyframes and
// combining each bone with its bind-pose offset, then writes the result straight into the same
// SSBO the PBR skinning shader reads (see include/animation.glsl). This replaces the CPU-side
// walk in Animation::calculateFinalBoneMatricesHelper + Animator::getFinalBoneMatrices for
// instanced entities. local_size_x is kept small (8) because each thread keeps a per-node
// transform array live for the whole walk - see MAX_NODES below.
layout (local_size_x = 8) in;

struct NodeData
{
    mat4 localBindTransform;
    int parentID;   // -1 for the root node
    int boneID;     // -1 if this node has no skinning influence
    uint posOffset;
    uint posCount;
    uint rotOffset;
    uint rotCount;
    uint scaleOffset;
    uint scaleCount;
};

struct InstanceParams
{
    uint outputBoneOffset; // mat4-unit offset into AnimationBuffer where this instance's bones start
    float currentTime;
};

layout(std430, binding = 3) readonly buffer NodeDataBuffer    { NodeData nodes[]; };
layout(std430, binding = 4) readonly buffer PosKeysBuffer     { vec4 posKeys[]; };    // xyz = position, w = time
layout(std430, binding = 5) readonly buffer ScaleKeysBuffer   { vec4 scaleKeys[]; };  // xyz = scale,    w = time
layout(std430, binding = 6) readonly buffer RotKeysBuffer     { vec4 rotKeys[]; };    // xyzw = quaternion
layout(std430, binding = 7) readonly buffer RotTimesBuffer    { float rotTimes[]; };  // parallel to rotKeys
layout(std430, binding = 8) readonly buffer BoneOffsetsBuffer { mat4 boneOffsets[]; };
layout(std430, binding = 9) readonly buffer InstanceParamsBuffer { InstanceParams instances[]; };

// Same physical buffer the PBR skinning shader reads (include/buffers.glsl, binding 1) - writable here.
layout(std430, binding = 1) buffer AnimationBuffer { mat4 animationBuffer[]; };

uniform uint nodeCount;
uniform uint instanceCount;

// Upper bound on skeleton size this kernel can hold per-thread. Mirrors the spirit of MAX_BONES
// in include/animation.glsl; nodes beyond this are silently skipped rather than overflowing.
const uint MAX_NODES = 128u;

mat4 translationMatrix(vec3 t)
{
    return mat4(
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        t.x, t.y, t.z, 1.0);
}

mat4 scaleMatrix(vec3 s)
{
    return mat4(
        s.x, 0.0, 0.0, 0.0,
        0.0, s.y, 0.0, 0.0,
        0.0, 0.0, s.z, 0.0,
        0.0, 0.0, 0.0, 1.0);
}

mat4 quatToMat4(vec4 q)
{
    float x = q.x, y = q.y, z = q.z, w = q.w;
    float x2 = x + x, y2 = y + y, z2 = z + z;
    float xx = x * x2, xy = x * y2, xz = x * z2;
    float yy = y * y2, yz = y * z2, zz = z * z2;
    float wx = w * x2, wy = w * y2, wz = w * z2;

    return mat4(
        1.0 - (yy + zz), xy + wz,         xz - wy,         0.0,
        xy - wz,         1.0 - (xx + zz), yz + wx,         0.0,
        xz + wy,         yz - wx,         1.0 - (xx + yy), 0.0,
        0.0,             0.0,             0.0,             1.0);
}

// Mirrors Bone::interpolatePosition / interpolateScale (Bone.cpp): find the last key at or
// before the current time and lerp towards the next one. Key counts per node are small enough
// that a linear scan beats the bookkeeping a binary search would need on the GPU.
vec3 sampleVec3Key(bool isScale, uint offset, uint count, float time)
{
    if (count == 0u)
    {
        return isScale ? vec3(1.0) : vec3(0.0);
    }
    if (count == 1u)
    {
        return isScale ? scaleKeys[offset].xyz : posKeys[offset].xyz;
    }

    uint last = offset + count - 1u;
    uint i0 = offset;
    for (uint i = offset; i < last; ++i)
    {
        float t = isScale ? scaleKeys[i].w : posKeys[i].w;
        if (t > time)
        {
            break;
        }
        i0 = i;
    }
    uint i1 = min(i0 + 1u, last);

    vec4 k0 = isScale ? scaleKeys[i0] : posKeys[i0];
    vec4 k1 = isScale ? scaleKeys[i1] : posKeys[i1];
    float progress = clamp((time - k0.w) / max(k1.w - k0.w, 1e-5), 0.0, 1.0);
    return mix(k0.xyz, k1.xyz, progress);
}

// GLSL has no built-in quaternion slerp - implement it directly on vec4, taking the short path.
vec4 slerp(vec4 a, vec4 b, float t)
{
    float cosHalfTheta = dot(a, b);
    if (cosHalfTheta < 0.0)
    {
        b = -b;
        cosHalfTheta = -cosHalfTheta;
    }
    if (cosHalfTheta > 0.9995)
    {
        return normalize(mix(a, b, t));
    }

    float halfTheta = acos(clamp(cosHalfTheta, -1.0, 1.0));
    float sinHalfTheta = sqrt(1.0 - cosHalfTheta * cosHalfTheta);
    float ratioA = sin((1.0 - t) * halfTheta) / sinHalfTheta;
    float ratioB = sin(t * halfTheta) / sinHalfTheta;
    return normalize(a * ratioA + b * ratioB);
}

// Mirrors Bone::interpolateRotation (Bone.cpp): slerp between the surrounding keys.
vec4 sampleRotationKey(uint offset, uint count, float time)
{
    if (count == 0u)
    {
        return vec4(0.0, 0.0, 0.0, 1.0);
    }
    if (count == 1u)
    {
        return normalize(rotKeys[offset]);
    }

    uint last = offset + count - 1u;
    uint i0 = offset;
    for (uint i = offset; i < last; ++i)
    {
        if (rotTimes[i] > time)
        {
            break;
        }
        i0 = i;
    }
    uint i1 = min(i0 + 1u, last);

    float progress = clamp((time - rotTimes[i0]) / max(rotTimes[i1] - rotTimes[i0], 1e-5), 0.0, 1.0);
    return normalize(slerp(normalize(rotKeys[i0]), normalize(rotKeys[i1]), progress));
}

void main()
{
    uint instanceIndex = gl_GlobalInvocationID.x;
    if (instanceIndex >= instanceCount)
    {
        return;
    }

    InstanceParams instanceParams = instances[instanceIndex];

    mat4 globalTransforms[MAX_NODES];

    uint clampedNodeCount = min(nodeCount, MAX_NODES);
    for (uint i = 0u; i < clampedNodeCount; ++i)
    {
        NodeData node = nodes[i];

        mat4 localTransform = node.localBindTransform;
        if (node.posCount > 0u || node.rotCount > 0u || node.scaleCount > 0u)
        {
            vec3 position = sampleVec3Key(false, node.posOffset, node.posCount, instanceParams.currentTime);
            vec4 rotation = sampleRotationKey(node.rotOffset, node.rotCount, instanceParams.currentTime);
            vec3 scale = sampleVec3Key(true, node.scaleOffset, node.scaleCount, instanceParams.currentTime);
            localTransform = translationMatrix(position) * quatToMat4(rotation) * scaleMatrix(scale);
        }

        mat4 globalTransform = (node.parentID < 0) ? localTransform : globalTransforms[node.parentID] * localTransform;
        globalTransforms[i] = globalTransform;

        if (node.boneID >= 0)
        {
            animationBuffer[instanceParams.outputBoneOffset + uint(node.boneID)] = globalTransform * boneOffsets[node.boneID];
        }
    }
}
