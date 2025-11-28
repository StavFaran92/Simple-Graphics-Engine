#vert

#version 330 

// ----- Definitions ----- //

#include ../../../../Engine/Resources/Engine/Shaders/include/defines.glsl
#include ../../../../Engine/Resources/Engine/Shaders/include/structs.glsl
#include ../../../../Engine/Resources/Engine/Shaders/include/uniforms.glsl
#include ../../../../Engine/Resources/Engine/Shaders/include/functions.glsl
                                                                                    
layout (location = 0) in vec3 aPos;
layout (location = 5) in ivec3 boneIDs;
layout (location = 6) in vec3 boneWeights;
layout (location = 7) in mat4 instanceModel;

#include ../../../../Engine/Resources/Engine/Shaders/include/animation.glsl

void main()
{
    mat4 aModel = model;

    if (isGpuInstanced)
    {
        aModel = model * instanceModel;
    }

    vec4 totalPosition;
    applySkinningPosition(aPos, boneIDs, boneWeights, totalPosition);

    gl_Position = projection * view * aModel * totalPosition;

}

#frag

#version 330 core

out vec4 FragColor;

void main() 
{
    FragColor = vec4(1.0); // white on black background
}