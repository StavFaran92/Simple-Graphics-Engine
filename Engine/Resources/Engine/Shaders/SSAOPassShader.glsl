#vert

#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec2 TexCoords;

void main()
{
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0); 
    TexCoords = aTexCoords;
}  

#frag

#version 330 

// ----- Definitions ----- //

#include ../../../../Engine/Resources/Engine/Shaders/include/defines.glsl
#include ../../../../Engine/Resources/Engine/Shaders/include/structs.glsl
#include ../../../../Engine/Resources/Engine/Shaders/include/uniforms.glsl
#include ../../../../Engine/Resources/Engine/Shaders/include/functions.glsl

// ----- In ----- //

in vec2 TexCoords;

// ----- Out ----- //

out float FragColor;

// ----- Uniforms ----- //

uniform int screenWidth;
uniform int screenHeight;
uniform vec3 ssaoKernel[64];

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gSSAONoise;

// ----- Forward Declerations ----- //

// ----- Methods ----- //
 
// ----- Variables ----- //
vec2 noiseScale = vec2(screenWidth/4.0, screenHeight/4.0);
float radius = 0.5;

void main() 
{ 
    // retrieve data from G-buffer
    vec3 fragPos = texture(gPosition, TexCoords).rgb;
    vec3 normal = texture(gNormal, TexCoords).rgb;
    vec3 ssaoNoise = texture(gSSAONoise, TexCoords * noiseScale).rgb;

    fragPos = vec3(view*vec4(fragPos, 1.0));
    normal = normalize(vec3(view*vec4(normal, 1.0)));

    // generate TBN
    vec3 tangent = normalize(ssaoNoise - normal * dot(ssaoNoise, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

    float occlusionFactor = 0.f;

    // take kernel
    for(int i=0; i<64; i++)
    {
        // transform sample by TBN and offset by fragPos
        vec3 samplePos = TBN * ssaoKernel[i];
        samplePos = fragPos + samplePos * radius;

        // take offset of sample and sample geometry position
        vec4 offset = vec4(samplePos, 1.0);
        offset = projection * offset;
        offset.xyz /= offset.w;
        offset.xyz = offset.xyz * .5 + .5;

        // Check if sample is inside geometry, if so add to occlusion factor
        float sampleDepth = texture(gPosition, offset.xy).z;

        vec3 offsetPos = texture(gPosition, offset.xy).xyz;
        sampleDepth = vec3(view*vec4(offsetPos, 1.0)).z;

        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
        occlusionFactor += (sampleDepth >= samplePos.z + 0.025 ? 1.0 : 0.0) * rangeCheck;  
    }

    occlusionFactor = 1.0 - (occlusionFactor / 64.0);
    FragColor = occlusionFactor;
} 