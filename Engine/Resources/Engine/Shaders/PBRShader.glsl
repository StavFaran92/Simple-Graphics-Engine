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

// ----- Out ----- //

out VS_OUT {
    vec3 fragPos;
    vec3 normal;
    vec2 texCoord;
} vs_out;

// ----- Uniforms ----- //

uniform mat4 finalBonesMatrices[MAX_BONES];
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
    mat4 aModel = model;

    if (isGpuInstanced)
    {
        aModel = model * instanceModel;
    }

    vec4 totalPosition = vec4(pos, 1.0f);
    vec3 totalNormal = norm;

    if (isAnimated)
    {
        totalPosition = vec4(0.0f);
        totalNormal = vec3(0.0f);

        for (int i = 0; i < MAX_BONE_INFLUENCE; i++)
        {
            if (boneIDs[i] == -1) continue;
            if (boneIDs[i] >= MAX_BONES)
            {
                totalPosition = vec4(pos, 1.0f);
                totalNormal = norm;
                break;
            }

            vec4 localPosition = finalBonesMatrices[boneIDs[i]] * vec4(pos, 1.0f);
            totalPosition += localPosition * boneWeights[i];
            vec3 localNormal = mat3(finalBonesMatrices[boneIDs[i]]) * norm;
            totalNormal += localNormal * boneWeights[i];
        }
    }

    vec3 aNorm = transposeInverseModelMatrix * totalNormal;

#ifdef CUSTOM_SHADER
    vert(totalPosition.xyz, aNorm);
#endif

    vs_out.texCoord = tex;
    vs_out.normal = aNorm;
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

const float MAX_REFLECTION_LOD = 4.0;

// ----- In ----- //

in VS_OUT {
    vec3 fragPos;
    vec3 normal;
    vec2 texCoord;
} fs_in;

// ----- Out ----- //

out vec4 FragColor;

// ----- Uniforms ----- //

uniform vec3 cameraPos;
uniform PBR_Material material;
uniform samplerCube gIrradianceMap;
uniform samplerCube gPrefilterEnvMap;
uniform sampler2D gBRDFIntegrationLUT;
uniform sampler2D gShadowMap;

// ----- Methods ----- //

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}   

float geometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

float geometrySmith(vec3 N, vec3 V, vec3 L, float k)
{
    float NdotV = max(0.0, dot(N, V));
    float NdotL = max(0.0, dot(N, L));
    return geometrySchlickGGX(NdotV, k) * geometrySchlickGGX(NdotL, k);
}

float distributionGGX(vec3 N, vec3 H, float a)
{
    float a2 = a * a;
    float NdotH = max(0.0, dot(N, H));
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = NdotH2 * (a2 - 1.0) + 1.0;
    denom = denom * denom * PI;

    return nom / denom;
}

struct Surface
{
    vec3 fragPos;
    vec3 V;
    vec3 N;
    vec3 H;
    vec3 L;
    vec3 F0;
    vec3 albedo;
    float metallic;
    float roughness;
};

vec3 calculateBRDF(Surface s)
{
    vec3 F = fresnelSchlick(max(0.0, dot(s.H, s.V)), s.F0);
    vec3 ks = F;
    vec3 kd = 1.0 - ks;
    kd *= 1.0 - s.metallic;

    float NDF = distributionGGX(s.N, s.H, s.roughness);
    float G = geometrySmith(s.N, s.V, s.L, s.roughness);

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(0.0, dot(s.N, s.V)) * max(0.0, dot(s.N, s.L)) + 0.0001;
    vec3 specular = numerator / denominator;

    return (specular + kd * s.albedo / PI);
}

vec3 PointLightRadiance(PointLight pLight, Surface s)
{
    s.L = normalize(pLight.position.rgb - s.fragPos);
    s.H = normalize(s.V + s.L);
    float distance = length(s.L);
    float attenuation = 1.0 / (distance * distance);
    vec3 radiance = pLight.color.rgb * attenuation;
    float cosTheta = max(0.0, dot(s.N, s.L));

    return calculateBRDF(s) * radiance * cosTheta;
}

vec3 DirLightRadiance(DirLight dLight, Surface s)
{
	s.L = normalize(-dLight.direction.xyz);
	s.H = normalize(s.V + s.L);

	// Calculate Li
	vec3 radiance = dLight.color.rgb;

	// Calculate cosTheta
	float cosTheta = max(0.0, dot(s.N, s.L));

	return calculateBRDF(s) * radiance * cosTheta;
}

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
    vec3 normal = normalize(fs_in.normal);
    vec3 albedo = pow(getPBRTexture(material.samplerAlbedo).rgb, vec3(2.2));
    float metallic = getPBRTexture(material.samplerMetallic).r;
    float roughness = getPBRTexture(material.samplerRoughness).r;
    float ao = getPBRTexture(material.samplerAO).r;

#ifdef CUSTOM_SHADER
    frag(albedo, normal, metallic, roughness, ao);
#endif

    vec3 V = normalize(cameraPos - fs_in.fragPos);
    vec3 R = reflect(-V, normal);

    vec3 F0 = mix(vec3(0.04), albedo, metallic);

    Surface s;
    s.fragPos = fs_in.fragPos;
    s.V = V;
    s.N = normal;
    s.F0 = F0;
    s.metallic = metallic;
    s.roughness = roughness;
    s.albedo = albedo;

    vec3 L0 = vec3(0.0);
    for (int i = 0; i < pointLightCount; ++i)
    {
        L0 += PointLightRadiance(pointLights[i], s);
    }

    for(int i = 0; i < dirLightCount; ++i)
	{
		L0 += DirLightRadiance(dirLight[i], s);
	}

    vec3 color = L0;

    FragColor = vec4(color, 1.0);
}
