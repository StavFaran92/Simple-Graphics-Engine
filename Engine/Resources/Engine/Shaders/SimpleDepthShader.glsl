#vert

#version 430 core
layout (location = 0) in vec3 pos;

layout (location = 5) in ivec3 boneIDs;
layout (location = 6) in vec3 boneWeights;

#include ../../../../Engine/Resources/Engine/Shaders/include/defines.glsl
#include ../../../../Engine/Resources/Engine/Shaders/include/structs.glsl
#include ../../../../Engine/Resources/Engine/Shaders/include/uniforms.glsl
#include ../../../../Engine/Resources/Engine/Shaders/include/functions.glsl
#include ../../../../Engine/Resources/Engine/Shaders/include/buffers.glsl
#include ../../../../Engine/Resources/Engine/Shaders/include/animation.glsl

// Mesh rest transform, constant for a whole instanced batch (one batch == one mesh)
uniform mat4 restTransform;

void main()
{
    mat4 finalModel = model;

    if (isGpuInstanced)
    {
        finalModel = transformBuffer[instanceOffset + gl_InstanceID] * restTransform;
    }

    vec4 totalPosition;
    applySkinningPosition(pos, boneIDs, boneWeights, isGpuInstanced, totalPosition);

    gl_Position = lightSpaceMatrix * finalModel * totalPosition;
}

#frag

#version 330 core

void main()
{             
    // gl_FragDepth = gl_FragCoord.z;
}  