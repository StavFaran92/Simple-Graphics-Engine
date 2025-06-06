#vert

#version 330

layout (location = 0) in vec3 pos;

// ----- Definitions ----- //

#include ../../../../Engine/Resources/Engine/Shaders/include/defines.glsl
#include ../../../../Engine/Resources/Engine/Shaders/include/structs.glsl
#include ../../../../Engine/Resources/Engine/Shaders/include/uniforms.glsl
#include ../../../../Engine/Resources/Engine/Shaders/include/functions.glsl

out vec3 localPos;

void main()
{
    localPos = pos;
    gl_Position = projection * view * vec4(pos, 1.0);
}

#frag

#version 330 

// ----- Definitions ----- //

#include ../../../../Engine/Resources/Engine/Shaders/include/defines.glsl
#include ../../../../Engine/Resources/Engine/Shaders/include/structs.glsl
#include ../../../../Engine/Resources/Engine/Shaders/include/uniforms.glsl
#include ../../../../Engine/Resources/Engine/Shaders/include/functions.glsl

in vec3 localPos;
out vec4 FragColor;

uniform samplerCube environmentMap;

void main()
{
    vec3 N = normalize(localPos);

    // generate TBN
    vec3 up = vec3(0.0, 1.0, 0.0);
    vec3 right  = normalize(cross(up, N));
    up          = normalize(cross( N, right));

    float sampleDelta = 0.025;
    vec3 irradiance = vec3(0.0, 0.0, 0.0);
    float numOfSamples = 0.0;

    // iterate 2PI of phi
    for(float phi = 0.0; phi < 2 * PI; phi += sampleDelta)
    {
        // iterate .5PI of theta (to create a hemisphere)
        for(float theta = 0.0; theta < .5 * PI; theta += sampleDelta)
        {
            // convert angles to cartesian coordinates (where Z is aligned with the normal)
            vec3 v = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));

            // convert from tangent space to world space using above TBN
            vec3 sampleVec = v.x * right + v.y * up + v.z * N;

            vec3 sample = texture(environmentMap, sampleVec).rgb;

            if(length(sample) > 10000)
            {
                continue;
            }

            // sample the evironemnt map 
            irradiance += texture(environmentMap, sampleVec).rgb * cos(theta) * sin(theta);
            numOfSamples++;
        }
    }

    // average irradiance
    irradiance = PI * irradiance * (1.0 / float(numOfSamples));

    FragColor = vec4(irradiance, 1.0);
}