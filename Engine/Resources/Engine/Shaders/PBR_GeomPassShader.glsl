#vert

#version 330

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 norm;
layout (location = 2) in vec2 tex;
layout (location = 5) in ivec3 boneIDs;
layout (location = 6) in vec3 boneWeights;

layout (location = 7) in mat4 instanceModel;

// ----- Definitions ----- //

#include ../../../../Engine/Resources/Engine/Shaders/include/defines.glsl
#include ../../../../Engine/Resources/Engine/Shaders/include/structs.glsl
#include ../../../../Engine/Resources/Engine/Shaders/include/uniforms.glsl
#include ../../../../Engine/Resources/Engine/Shaders/include/functions.glsl
#include ../../../../Engine/Resources/Engine/Shaders/include/animation.glsl

// ----- Structs ----- //

// ----- Out ----- //

out VS_OUT {
    vec3 fragPos;
	vec3 normal;
    vec2 texCoord;
    vec3 fragPosVS;
	vec3 normalVS;
} vs_out;

// ----- Forward Declerations ----- //

// ----- Methods ----- //

float getTime()
{
	return iTime;
}

#ifdef CUSTOM_SHADER
#custom_vert
#endif

void main()
{
	mat4 aModel = model;

	if(isGpuInstanced)
	{
		aModel = model * instanceModel;
	}

        vec4 totalPosition;
        vec3 totalNormal;
        applySkinning(pos, norm, boneIDs, boneWeights, totalPosition, totalNormal);

	vec3 aNorm = mat3(transpose(inverse(aModel))) * totalNormal;
#ifdef CUSTOM_SHADER
	vert(totalPosition.xyz, aNorm);
#endif

	vs_out.texCoord = tex;
	vs_out.normal =  aNorm;
	vs_out.fragPos = (aModel * totalPosition).xyz;
	vs_out.fragPosVS = (view * vec4(vs_out.fragPos,1.0)).xyz;
	vs_out.normalVS = (view * vec4(vs_out.normal,0.0)).xyz;

	gl_Position = projection * view * aModel * totalPosition;
}

#frag

#version 330 

// ----- Definitions ----- //

#include ../../../../Engine/Resources/Engine/Shaders/include/defines.glsl
#include ../../../../Engine/Resources/Engine/Shaders/include/structs.glsl
#include ../../../../Engine/Resources/Engine/Shaders/include/uniforms.glsl
#include ../../../../Engine/Resources/Engine/Shaders/include/functions.glsl

// ----- In ----- //

in VS_OUT {
    vec3 fragPos;
	vec3 normal;
    vec2 texCoord;
    vec3 fragPosVS;
	vec3 normalVS;
} fs_in;

// ----- Out ----- //

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec3 gAlbedo;
layout (location = 3) out vec3 gMRA;
layout (location = 4) out vec3 gPositionVS;
layout (location = 5) out vec3 gNormalVS;


// ----- Uniforms ----- //
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

// ----- Forward Declerations ----- //

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
 
void main() 
{ 	
	gPosition = fs_in.fragPos;
	gNormal = normalize(fs_in.normal) * getPBRTexture(samplerNormal).rgb;
	gAlbedo = getPBRTexture(samplerAlbedo).rgb * color;
	gMRA.r = getPBRTexture(samplerMetallic).r * metallicFactor;
	gMRA.g = getPBRTexture(samplerRoughness).r * roughnessFactor;
	gMRA.b = getPBRTexture(samplerAO).r;
	gPositionVS = fs_in.fragPosVS;
	gNormalVS = normalize(fs_in.normalVS);
} 