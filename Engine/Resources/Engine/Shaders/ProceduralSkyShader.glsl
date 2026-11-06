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

// Linear colors; the skybox pass and the lit shaders apply gamma
const vec3 skyTopColor = vec3(0.18, 0.38, 0.80);
const vec3 skyHorizonColor = vec3(0.65, 0.76, 0.90);
const vec3 groundHorizonColor = vec3(0.42, 0.40, 0.38);
const vec3 groundBottomColor = vec3(0.14, 0.13, 0.12);
const float skyCurve = 0.45;    // below 1 keeps the horizon color low
const float groundCurve = 0.25;

void main()
{
    vec3 dir = normalize(localPos);

    vec3 sky    = mix(skyHorizonColor,    skyTopColor,       pow(max(dir.y, 0.0),  skyCurve));
    vec3 ground = mix(groundHorizonColor, groundBottomColor, pow(max(-dir.y, 0.0), groundCurve));

    float horizonBlend = 0.05; // width of the fade; bigger = softer
    float t = smoothstep(-horizonBlend, horizonBlend, dir.y);

    vec3 color = mix(ground, sky, t);

    FragColor = vec4(color, 1.0);
}
