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

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 3;

// ----- Structs ----- //

// ----- Out ----- //

out VS_OUT {
    vec3 fragPos;
	vec3 normal;
    vec2 texCoord;
} vs_out;

// ----- Uniforms ----- //

uniform mat4 finalBonesMatrices[MAX_BONES];

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

	vec4 totalPosition = vec4(pos, 1.0f);
	vec3 totalNormal = norm;
	
	if(isAnimated)
	{
		totalPosition = vec4(0.0f);
		totalNormal = vec3(0.0f);

		for(int i = 0 ; i < MAX_BONE_INFLUENCE ; i++)
		{
			if(boneIDs[i] == -1) 
				continue;
			if(boneIDs[i] >=MAX_BONES) 
			{
				totalPosition = vec4(pos,1.0f);
				totalNormal = norm;
				break;
			}

			vec4 localPosition = finalBonesMatrices[boneIDs[i]] * vec4(pos,1.0f);
			totalPosition += localPosition * boneWeights[i];
			vec3 localNormal = mat3(finalBonesMatrices[boneIDs[i]]) * norm;
			totalNormal += localNormal * boneWeights[i];
		}
	}

	vec3 aNorm = mat3(transpose(inverse(aModel))) * totalNormal;
#ifdef CUSTOM_SHADER
	vert(totalPosition.xyz, aNorm);
#endif

	vs_out.texCoord = tex;
	vs_out.normal =  aNorm;
	vs_out.fragPos = (aModel * totalPosition).xyz;

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
} fs_in;

// ----- Out ----- //

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec3 gAlbedo;
layout (location = 3) out vec3 gMRA;

// ----- Uniforms ----- //
uniform PBR_Material material; 

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
	gNormal = normalize(fs_in.normal) * getPBRTexture(material.samplerNormal).rgb;
	gAlbedo = getPBRTexture(material.samplerAlbedo).rgb * material.colorDiffuse;
	gMRA.r = getPBRTexture(material.samplerMetallic).r * material.metallicFactor;
	gMRA.g = getPBRTexture(material.samplerRoughness).r * material.roughnessFactor;
	gMRA.b = getPBRTexture(material.samplerAO).r;
} 