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

// About 35 degrees above the horizon
const vec3 sunDirection = vec3(0.58, 0.574, 0.58);
const vec3 sunColor = vec3(12.0, 11.0, 9.0);
const float sunAngle = 1.2;     // disk radius, degrees
const float sunGlowAngle = 12.0;

void main()
{
    vec3 dir = normalize(localPos);

    vec3 color;
    if (dir.y >= 0.0)
        color = mix(skyHorizonColor, skyTopColor, pow(dir.y, skyCurve));
    else
        color = mix(groundHorizonColor, groundBottomColor, pow(-dir.y, groundCurve));

    float cosToSun = dot(dir, normalize(sunDirection));
    float angleToSun = degrees(acos(clamp(cosToSun, -1.0, 1.0)));

    // Soft edge so the disk doesn't alias in the cubemap
    float disk = 1.0 - smoothstep(sunAngle * 0.7, sunAngle, angleToSun);
    float glow = pow(1.0 - smoothstep(0.0, sunGlowAngle, angleToSun), 2.0) * 0.35;

    // No sun below the horizon
    float aboveHorizon = step(0.0, dir.y);
    color += (sunColor * disk + skyHorizonColor * glow) * aboveHorizon;

    FragColor = vec4(color, 1.0);
}
