// ----- Structs ----- //

struct Material 
{ 
	// textures 
	sampler2D texture_diffuse; 
	sampler2D texture_specular; 
	sampler2D texture_normal;
}; 

struct PBR_Sampler
{
	sampler2D texture;
	int channelMaskR;
	int channelMaskG;
	int channelMaskB;
	int channelMaskA;
	float xOffset;
	float yOffset;
	float xScale;
	float yScale;
};

struct PBR_Material 
{ 
	PBR_Sampler samplerAlbedo;
	PBR_Sampler samplerNormal;
	PBR_Sampler samplerMetallic;
	PBR_Sampler samplerRoughness;
	PBR_Sampler samplerAO;
	float roughnessFactor;
	float metallicFactor;
	vec3 colorDiffuse;
	float opacityFactor;
}; 

struct PointLight 
{ 
	vec4 position; 
	vec4 color; 
}; 

struct DirLight 
{ 
	vec4 direction; 
	vec4 color; 
}; 