#vert

#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 norm;
layout (location = 2) in vec2 tex;

out vec2 TexCoords;
out vec2 ScreenUV;

void main()
{
    TexCoords = tex;
	ScreenUV = vec2(aPos.x,aPos.y) ;

    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0); 
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

in vec2 TexCoords;
in vec2 ScreenUV;

// ----- Out ----- //

out vec4 FragColor;

// ----- Uniforms ----- //

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D gMRA;
uniform samplerCube gIrradianceMap;
uniform samplerCube gPrefilterEnvMap;
uniform sampler2D gBRDFIntegrationLUT;
uniform sampler2D gShadowMap;
uniform vec3 cameraPos;
uniform sampler2D gSSAOColorBuffer;
uniform sampler2D gTangent;

uniform bool useSSAO;

// ----- Forward Declerations ----- //

// ----- Methods ----- //

float getPixelDepth()
{
	return length(cameraPos - texture(gPosition, TexCoords).rgb);
}

vec2 getScreenUV()
{
	return ScreenUV;
}

vec3 getPixelPosition()
{
	return texture(gPosition, TexCoords).rgb;
}

vec3 getPixelNormal()
{
	return texture(gNormal, TexCoords).rgb;
}

vec3 getCameraPosition()
{
	return cameraPos;
}

vec2 getTexCoords()
{
	return TexCoords;
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
	// retrieve data from G-buffer
	vec3 fragPos = texture(gPosition, TexCoords).rgb;
	vec3 normal = normalize(texture(gNormal, TexCoords).rgb);
	vec3 albedo = pow(texture(gAlbedo, TexCoords).rgb, vec3(2.2));
	float metallic = texture(gMRA, TexCoords).r;
	float roughness = texture(gMRA, TexCoords).g;
	float ao = texture(gMRA, TexCoords).b;
	vec3 tangent = normalize(texture(gTangent, TexCoords).rgb);
	float ssao = texture(gSSAOColorBuffer, TexCoords).r;

	vec4 fragPosInLightSpace = lightSpaceMatrix * vec4(fragPos, 1.f);
	float shadow = calculateShadows(fragPosInLightSpace, gShadowMap);

	float ssaoFinal = useSSAO ? ao * ssao : 1.0;

	vec3 color = calculatePBR(
		albedo, 
		normal, 
		metallic, 
		roughness, 
		ssaoFinal, 
		cameraPos, 
		fragPos,
		shadow,
		gPrefilterEnvMap, 
		gIrradianceMap, 
		gBRDFIntegrationLUT);

	// HDR
	color = color / (color + vec3(1.0));

	// Gamma correction
	color = pow(color, vec3(1.0/2.2));

#ifdef CUSTOM_SHADER
	frag(color);
#endif
    
    FragColor = vec4(color, 1.0);
} 