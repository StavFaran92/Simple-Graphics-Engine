#vert

#version 410 core
                                                                                    
layout (location = 0) in vec3 aPos;  
layout (location = 2) in vec2 aTexCoords;                                        

out vec2 texCoords;
out mat3 TBN;

uniform int width;
uniform int height;
                                                            
void main()                                                                         
{
    texCoords = aTexCoords;
    vec3 pos = aPos.xyz;
    pos *= vec3(width, 0, height);
    gl_Position = vec4(pos, 1.0);                                       
}

#tesc

#version 410 core

// Number of vertices in a patch
layout (vertices=4) out;

uniform mat4 model;
uniform mat4 view;

in vec2 texCoords[];

out vec2 outTexCoords[];

const int MIN_TESS_LEVEL = 4;
const int MAX_TESS_LEVEL = 64;
const float MIN_DISTANCE = 20;
const float MAX_DISTANCE = 200;

void main()
{
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    outTexCoords[gl_InvocationID] = texCoords[gl_InvocationID];

    //If we are in Vertex 0 in the patch
    if(gl_InvocationID == 0)
    {
        // transform each vertex into eye space
        vec4 eyeSpacePos00 = view * model * gl_in[0].gl_Position;
        vec4 eyeSpacePos01 = view * model * gl_in[1].gl_Position;
        vec4 eyeSpacePos10 = view * model * gl_in[2].gl_Position;
        vec4 eyeSpacePos11 = view * model * gl_in[3].gl_Position;

        // "distance" from camera scaled between 0 and 1
        float distance00 = clamp((abs(eyeSpacePos00.z)-MIN_DISTANCE) / (MAX_DISTANCE-MIN_DISTANCE), 0.0, 1.0);
        float distance01 = clamp((abs(eyeSpacePos01.z)-MIN_DISTANCE) / (MAX_DISTANCE-MIN_DISTANCE), 0.0, 1.0);
        float distance10 = clamp((abs(eyeSpacePos10.z)-MIN_DISTANCE) / (MAX_DISTANCE-MIN_DISTANCE), 0.0, 1.0);
        float distance11 = clamp((abs(eyeSpacePos11.z)-MIN_DISTANCE) / (MAX_DISTANCE-MIN_DISTANCE), 0.0, 1.0);

        // interpolate edge tessellation level based on closer vertex
        float tessLevel0 = mix( MAX_TESS_LEVEL, MIN_TESS_LEVEL, min(distance10, distance00) );
        float tessLevel1 = mix( MAX_TESS_LEVEL, MIN_TESS_LEVEL, min(distance00, distance01) );
        float tessLevel2 = mix( MAX_TESS_LEVEL, MIN_TESS_LEVEL, min(distance01, distance11) );
        float tessLevel3 = mix( MAX_TESS_LEVEL, MIN_TESS_LEVEL, min(distance11, distance10) );

        // set the corresponding outer edge tessellation levels
        gl_TessLevelOuter[0] = tessLevel0;
        gl_TessLevelOuter[1] = tessLevel1;
        gl_TessLevelOuter[2] = tessLevel2;
        gl_TessLevelOuter[3] = tessLevel3;

        // set the inner tessellation levels to the max of the two parallel edges
        gl_TessLevelInner[0] = max(tessLevel1, tessLevel3);
        gl_TessLevelInner[1] = max(tessLevel0, tessLevel2);
    }
}

#tese

#version 410 core

layout (quads, fractional_odd_spacing, ccw) in;

uniform sampler2D heightMap;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float scale;

in vec2 outTexCoords[];

out vec2 texCoord;

// Send to fragment shader for coloring
out float height;
out vec3 fragPos;
out vec3 fragNormal;
out vec3 tangent;
out vec3 bitangent;

void main()
{
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;

    // Interpolate UVs across the patch
    vec2 t00 = outTexCoords[0];
    vec2 t01 = outTexCoords[1];
    vec2 t10 = outTexCoords[2];
    vec2 t11 = outTexCoords[3];

    vec2 t0 = mix(t00, t01, u);
    vec2 t1 = mix(t10, t11, u);
    vec2 texCoordLocal = mix(t0, t1, v);

    // Clamp to valid texel range
    vec2 texSize = textureSize(heightMap, 0);
    vec2 texel   = 1.0 / texSize;
    vec2 halfTexel = 0.5 * texel;
    texCoord = clamp(texCoordLocal, halfTexel, 1.0 - halfTexel);

    // Sample height and neighbors (central differences)
    float h  = texture(heightMap, texCoord).r;
    float hL = texture(heightMap, texCoord - vec2(texel.x, 0)).r;
    float hR = texture(heightMap, texCoord + vec2(texel.x, 0)).r;
    float hD = texture(heightMap, texCoord - vec2(0, texel.y)).r;
    float hU = texture(heightMap, texCoord + vec2(0, texel.y)).r;

    // Height derivatives
    float dhdx = (hR - hL) * 0.5;
    float dhdz = (hU - hD) * 0.5;

    // Patch position interpolation (object space)
    vec4 p00 = gl_in[0].gl_Position;
    vec4 p01 = gl_in[1].gl_Position;
    vec4 p10 = gl_in[2].gl_Position;
    vec4 p11 = gl_in[3].gl_Position;

    vec4 p0 = mix(p00, p01, u);
    vec4 p1 = mix(p10, p11, u);
    vec4 p  = mix(p0, p1, v);

    // Displace vertex
    p.y += h * scale;

    // Build correct TBN for height surface y = f(x, z)
    vec3 T = vec3(1.0, dhdx * scale, 0.0); // ∂P/∂x
    vec3 B = vec3(0.0, dhdz * scale, 1.0); // ∂P/∂z

    vec3 N = normalize(cross(B, T));

    // Orthonormalize (important!)
    T = normalize(T - N * dot(N, T));
    B = normalize(cross(N, T));

    // Transform to world space
    mat3 normalMatrix = mat3(transpose(inverse(model)));

    fragNormal = normalize(normalMatrix * N);
    tangent    = normalize(normalMatrix * T);
    bitangent  = normalize(normalMatrix * B);

    fragPos = (model * p).xyz;
    gl_Position = projection * view * model * p;
}

#frag

#version 410 core

#include ../../../../Engine/Resources/Engine/Shaders/include/defines.glsl
#include ../../../../Engine/Resources/Engine/Shaders/include/structs.glsl
#include ../../../../Engine/Resources/Engine/Shaders/include/uniforms.glsl
#include ../../../../Engine/Resources/Engine/Shaders/include/functions.glsl
#include ../../../../Engine/Resources/Engine/Shaders/include/PBR.glsl
#include ../../../../Engine/Resources/Engine/Shaders/include/Shadows.glsl

uniform int textureCount;

uniform sampler2D texture_0;
uniform sampler2D texture_1;
uniform sampler2D texture_2;
uniform sampler2D texture_3;

uniform float textureBlend[4];
uniform vec2 textureScale[4];

uniform float scale;

uniform vec3 cameraPos;
uniform samplerCube gIrradianceMap;
uniform samplerCube gPrefilterEnvMap;
uniform sampler2D gBRDFIntegrationLUT;
uniform sampler2D gShadowMap;

#pragma editable
uniform PBR_Sampler samplerAlbedo;

#pragma editable
uniform PBR_Sampler samplerNormal;

#pragma editable
uniform PBR_Sampler samplerRoughness;

#pragma editable
uniform PBR_Sampler samplerMetallic;

#pragma editable
uniform PBR_Sampler samplerAO;

#pragma editable (default=1.0)
uniform float roughnessFactor;

#pragma editable (default=0.0)
uniform float metallicFactor;

#pragma editable (default=0.0)
uniform float aoFactor;

#pragma editable (default=(1.0, 1.0, 1.0))
uniform vec3 color;

#pragma editable (default=(1.0, 1.0))
uniform vec2 globalUV;

in float height;
in vec2 texCoord;
in vec3 fragPos;
in vec3 fragNormal;
in vec3 tangent;
in vec3 bitangent;

out vec4 FragColor;

vec4 sampleFromTexture(int textureIndex, vec2 uv)
{
    uv *= textureScale[textureIndex];

    if (textureIndex == 0)  return texture(texture_0, uv);
    else if (textureIndex == 1) return texture(texture_1, uv);
    else if (textureIndex == 2) return texture(texture_2, uv);
    else if (textureIndex == 3) return texture(texture_3, uv);

    return vec4(0.0); // Return black if index is out of bounds
}

void main()
{
    mat3 TBN = mat3(tangent, bitangent, fragNormal);
    vec3 normal;
    vec3 albedo;
    vec3 MRA;
    samplePBR(
		// Input
		TBN,
		fragNormal,
		color,
		metallicFactor,
		roughnessFactor,
        texCoord * globalUV,
        
		samplerNormal,
		samplerAlbedo,
		samplerMetallic, // todo fix
		samplerRoughness,
		samplerAO,

		// Output
		normal,
		albedo,
		MRA
	);

    vec4 fragPosInLightSpace = lightSpaceMatrix * vec4(fragPos, 1.f);
    float shadow = calculateShadows(fragPosInLightSpace, gShadowMap);

    vec3 color = calculatePBR(
                albedo,
                normal,
                MRA.r,
                MRA.g,
                MRA.b,
                cameraPos,
                fragPos,
                shadow,
                gPrefilterEnvMap,
                gIrradianceMap,
                gBRDFIntegrationLUT);

    // HDR tonemapping
    color = color / (color + vec3(1.0));
    // gamma correct
    color = pow(color, vec3(1.0/2.2));

    FragColor = vec4(color, 1.);
    // FragColor = vec4(normalSample, 1.);

    // if(height < textureBlend[0])
    // {
    //     color = sampleFromTexture(0, texCoord);
    // }
    // else if(height >= textureBlend[textureCount - 1])
    // {
    //     color = sampleFromTexture(textureCount - 1, texCoord);
    // }
    // else
    // {
    //     for(int i=0; i < textureCount - 1; i++)
    //     {
    //         if(height >= textureBlend[i] && height < textureBlend[i + 1])
    //         {
    //             float b1 = height - textureBlend[i];
    //             float b2  = textureBlend[i + 1] - height;
    //             float blend = b1 / (b1 + b2);
    //             color = mix(sampleFromTexture(i, texCoord), sampleFromTexture(i + 1, texCoord), blend);
    //             break;
    //         }
    //     }
    // }

    // vec4 fragPosInLightSpace = lightSpaceMatrix * vec4(fragPos, 1.f);
	// float shadow = shadowCalculations(fragPosInLightSpace) * 0.8; // 0.8 to generate some ambient light

    // color = color * (1.0 - shadow);
}