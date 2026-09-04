#vert

#version 330

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 4) in vec4 aTangent;
layout (location = 5) in ivec3 aBoneIDs;
layout (location = 6) in vec3 aBoneWeights;

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
	vec4 tangent;
	mat3 TBN;
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
	mat4 finalModel = model;

    // if (isGpuInstanced)
    // {
    //     finalModel = model * instanceModel;
    // }

	vec4 totalPosition;
	vec3 totalNormal;
	applySkinning(aPos, aNormal, aBoneIDs, aBoneWeights, totalPosition, totalNormal);

	vec3 normWS = mat3(transpose(inverse(finalModel))) * totalNormal;

#ifdef CUSTOM_SHADER
	vert(totalPosition.xyz, normWS);
#endif

	vec3 tangent = aTangent.xyz;

	vec3 bitangent = cross(normalize(totalNormal), tangent) * aTangent.w;

	vec3 T = normalize(vec3(finalModel * vec4(tangent, 0.f)));
	vec3 B = normalize(vec3(finalModel * vec4(bitangent, 0.f)));
	vec3 N = normalize(normWS);
	vs_out.TBN = mat3(T, B, N);

	vs_out.texCoord = aTexCoord;
	vs_out.normal =  normWS;
	vs_out.fragPos = (finalModel * totalPosition).xyz;
	vs_out.fragPosVS = (view * vec4(vs_out.fragPos,1.0)).xyz;
	vs_out.normalVS = (view * vec4(vs_out.normal,0.0)).xyz;
	vs_out.tangent = aTangent; // tangent is only direction ?

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

// ----- In ----- //

in VS_OUT {
    vec3 fragPos;
	vec3 normal;
    vec2 texCoord;
    vec3 fragPosVS;
	vec3 normalVS;
	vec4 tangent;
	mat3 TBN;
} fs_in;

// ----- Out ----- //

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec3 gAlbedo;
layout (location = 3) out vec3 gMRA;
layout (location = 4) out vec3 gPositionVS;
layout (location = 5) out vec3 gNormalVS;
layout (location = 6) out vec3 gTangent;


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
 
void main() 
{ 	
	gPosition = fs_in.fragPos;

	samplePBR(
		// Input
		fs_in.TBN,
		fs_in.normal,
		color,
		metallicFactor,
		roughnessFactor,
		fs_in.texCoord,

		samplerNormal,
		samplerAlbedo,
		samplerMetallic,
		samplerRoughness,
		samplerAO,

		// Output
		gNormal,
		gAlbedo,
		gMRA
	);

	gPositionVS = fs_in.fragPosVS;
	gNormalVS = normalize(fs_in.normalVS);
} 