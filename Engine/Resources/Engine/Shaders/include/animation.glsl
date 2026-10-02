const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 3;

uniform mat4 finalBonesMatrices[MAX_BONES];

void applySkinning(
    vec3 pos,
    vec3 norm,
    ivec3 boneIDs,
    vec3 boneWeights,
    bool gpuInstanced,
    out vec4 totalPosition,
    out vec3 totalNormal)
{
    totalPosition = vec4(pos, 1.0f);
    totalNormal = norm;

    bool animated = isAnimated;
    int boneOffset = 0;

    if (gpuInstanced)
    {
        InstanceData instData = instanceDataBuffer[gl_InstanceID];
        animated = instData.isAnimated != 0u;
        boneOffset = int(instData.modelIndex);
    }

    if (animated)
    {
        totalPosition = vec4(0.0f);
        totalNormal = vec3(0.0f);

        for (int i = 0; i < MAX_BONE_INFLUENCE; i++)
        {
            if (boneIDs[i] == -1) continue;
            if (boneIDs[i] >= MAX_BONES)
            {
                totalPosition = vec4(pos, 1.0f);
                totalNormal = norm;
                break;
            }

            mat4 boneMatrix = gpuInstanced ? animationBuffer[boneOffset + boneIDs[i]] : finalBonesMatrices[boneIDs[i]];

            vec4 localPosition = boneMatrix * vec4(pos, 1.0f);
            totalPosition += localPosition * boneWeights[i];
            vec3 localNormal = mat3(boneMatrix) * norm;
            totalNormal += localNormal * boneWeights[i];
        }
    }
}

void applySkinningPosition(vec3 pos, ivec3 boneIDs, vec3 boneWeights, bool gpuInstanced, out vec4 totalPosition)
{
    vec3 unusedNormal;
    applySkinning(pos, vec3(0.0f), boneIDs, boneWeights, gpuInstanced, totalPosition, unusedNormal);
}
