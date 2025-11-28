#vert

#version 330 core
layout (location = 0) in vec3 pos;

layout (location = 5) in ivec3 boneIDs;
layout (location = 6) in vec3 boneWeights;

#include ../../../../Engine/Resources/Engine/Shaders/include/defines.glsl
#include ../../../../Engine/Resources/Engine/Shaders/include/structs.glsl
#include ../../../../Engine/Resources/Engine/Shaders/include/uniforms.glsl
#include ../../../../Engine/Resources/Engine/Shaders/include/functions.glsl
#include ../../../../Engine/Resources/Engine/Shaders/include/animation.glsl

void main()
{
    vec4 totalPosition;
    applySkinningPosition(pos, boneIDs, boneWeights, totalPosition);

    gl_Position = lightSpaceMatrix * model * totalPosition;
}

#frag

#version 330 core

void main()
{             
    // gl_FragDepth = gl_FragCoord.z;
}  