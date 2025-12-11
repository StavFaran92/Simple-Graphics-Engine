#vert

#version 330

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 4) in vec3 aTangent;
layout (location = 5) in ivec3 aBoneIDs;
layout (location = 6) in vec3 aBoneWeights;
layout (location = 7) in mat4 instanceModel;

// ----- Definitions ----- //

#include ../../../../Engine/Resources/Engine/Shaders/include/defines.glsl
#include ../../../../Engine/Resources/Engine/Shaders/include/structs.glsl
#include ../../../../Engine/Resources/Engine/Shaders/include/uniforms.glsl
#include ../../../../Engine/Resources/Engine/Shaders/include/functions.glsl
#include ../../../../Engine/Resources/Engine/Shaders/include/animation.glsl

// ----- Out ----- //

out VS_OUT {
    vec3 fragPos;
    vec3 normal;
    vec2 texCoord;
    mat3 TBN;
} vs_out;

uniform mat3 transposeInverseModelMatrix;

float getTime()
{
	return iTime;
}

#ifdef CUSTOM_SHADER
#custom_vert
#endif

void main()
{
    mat4 finalModel = model;

    if (isGpuInstanced)
    {
        finalModel = model * instanceModel;
    }

    vec4 totalPosition;
    vec3 totalNormal;
    applySkinning(aPos, aNormal, aBoneIDs, aBoneWeights, totalPosition, totalNormal);

    vec3 normWS = mat3(transpose(inverse(model))) * totalNormal;

#ifdef CUSTOM_SHADER
    vert(totalPosition.xyz, normWS);
#endif

    vec3 bitangent = cross(normalize(totalNormal), aTangent);

	vec3 T = normalize(vec3(model * vec4(aTangent, 0.f)));
	vec3 B = normalize(vec3(model * vec4(bitangent, 0.f)));
	vec3 N = normalize(normWS);
	vs_out.TBN = mat3(T, B, N);

    vs_out.texCoord = aTexCoord;
    vs_out.normal = normWS;
    vs_out.fragPos = (finalModel * totalPosition).xyz;

    gl_Position = projection * view * finalModel * totalPosition;
}

#frag

#version 330

// ----- Definitions ----- //

#include ../../../../Engine/Resources/Engine/Shaders/include/defines.glsl
#include ../../../../Engine/Resources/Engine/Shaders/include/structs.glsl
#include ../../../../Engine/Resources/Engine/Shaders/include/uniforms.glsl
#include ../../../../Engine/Resources/Engine/Shaders/include/functions.glsl
#include ../../../../Engine/Resources/Engine/Shaders/include/PBR.glsl
#include ../../../../Engine/Resources/Engine/Shaders/include/Shadows.glsl

// ----- In ----- //

in VS_OUT {
    vec3 fragPos;
    vec3 normal;
    vec2 texCoord;
    mat3 TBN;
} fs_in;

// ----- Out ----- //

out vec4 FragColor;

// ----- Uniforms ----- //

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
uniform PBR_Sampler samplerMetallic;

#pragma editable
uniform PBR_Sampler samplerRoughness;

#pragma editable
uniform PBR_Sampler samplerAO;

#pragma editable (default=1.0)
uniform float roughnessFactor;

#pragma editable (default=0.0)
uniform float metallicFactor;

#pragma editable (default=(1.0, 1.0, 1.0))
uniform vec3 color;

#pragma editable (default=1.0)
uniform float opacityFactor;

// ----- Methods ----- //

#define CHANNEL_NONE 0
#define CHANNEL_R 1
#define CHANNEL_G 2
#define CHANNEL_B 3
#define CHANNEL_A 4

float extractChannel(vec4 inputColor, int channelMask) 
{
    if (channelMask == CHANNEL_NONE) return 0.f;
    if (channelMask == CHANNEL_R) return inputColor.r;
    if (channelMask == CHANNEL_G) return inputColor.g;
    if (channelMask == CHANNEL_B) return inputColor.b;
    if (channelMask == CHANNEL_A) return inputColor.a;

    return 0;
}

vec4 getPBRTexture(PBR_Sampler s)
{
	vec4 color = texture(s.texture, fs_in.texCoord * vec2(s.xScale, s.yScale) + vec2(s.xOffset, s.yOffset)).rgba;
	return vec4(extractChannel(color, s.channelMaskR), 
				extractChannel(color, s.channelMaskG), 
				extractChannel(color, s.channelMaskB), 
				extractChannel(color, s.channelMaskA));
}

float getPixelDepth()
{
	return length(cameraPos - fs_in.fragPos);
}

//vec2 getScreenUV()
//{
//	return ScreenUV;
//}

vec3 getPixelPosition()
{
	return fs_in.fragPos;
}

vec3 getPixelNormal()
{
	return fs_in.normal;
}

vec3 getCameraPosition()
{
	return cameraPos;
}

vec2 getTexCoords()
{
	return fs_in.texCoord;
}

float getTime()
{
	return iTime;
}

#ifdef CUSTOM_SHADER
#custom_frag
#endif

void main()
{
    vec3 normal = samplerNormal.isActive ? (fs_in.TBN * getPBRTexture(samplerNormal).rgb * 2.0 - 1.0) : fs_in.normal;
    normal = normalize(normal);

    vec3 albedoTex = samplerAlbedo.isActive ? getPBRTexture(samplerAlbedo).rgb : vec3(1.0);
	vec3 albedo = albedoTex * color;
	albedo = pow(albedo, vec3(2.2)); // Gamme Correction

	float metallicTex = samplerMetallic.isActive ? getPBRTexture(samplerMetallic).r : 1.0;
	float metallic = metallicTex * metallicFactor;

	float roughnessTex = samplerRoughness.isActive ? getPBRTexture(samplerRoughness).r : 1.0;
	float roughness = roughnessTex * roughnessFactor;

	float aoTex = samplerAO.isActive ? getPBRTexture(samplerAO).r : 1.0;
	float ao = aoTex;

#ifdef CUSTOM_SHADER
    frag(albedo, normal, metallic, roughness, ao);
#endif

    float ssaoFinal = ao;

    vec3 color = calculatePBR(
		albedo, 
		normal, 
		metallic, 
		roughness, 
		ssaoFinal, 
		cameraPos, 
		fs_in.fragPos,
		1.f,
		gPrefilterEnvMap, 
		gIrradianceMap, 
		gBRDFIntegrationLUT);

    // HDR tonemapping
    color = color / (color + vec3(1.0));
    // gamma correct
    color = pow(color, vec3(1.0/2.2));

    // FragColor = vec4(normal, opacityFactor);
    FragColor = vec4(color, opacityFactor);
}
