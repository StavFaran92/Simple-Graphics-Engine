// Must match MAX_SHADOW_CASCADES in ShadowSettings.h
#define MAX_SHADOW_CASCADES 4

uniform mat4 lightSpaceMatrices[MAX_SHADOW_CASCADES];
uniform float cascadeSplits[MAX_SHADOW_CASCADES]; // far view-space distance of each cascade
uniform float cascadeBias[MAX_SHADOW_CASCADES];
uniform int cascadeCount;

// TODO: sampler2DArrayShadow + GL_TEXTURE_COMPARE_MODE, each tap becomes a hardware 2x2 PCF

// viewDepth: positive distance along the camera's forward axis. Returns 1 when fully shadowed
float calculateShadows(vec3 worldPos, float viewDepth, sampler2DArray shadowMap)
{
	int layer = -1;
	for (int i = 0; i < cascadeCount; ++i)
	{
		if (viewDepth < cascadeSplits[i])
		{
			layer = i;
			break;
		}
	}

	// Past the shadow distance
	if (layer == -1)
		return 0.0;

	vec4 fragPos = lightSpaceMatrices[layer] * vec4(worldPos, 1.0);
	vec3 projCoords = fragPos.xyz / fragPos.w * 0.5 + 0.5;

	vec2 texelSize = 1.0 / vec2(textureSize(shadowMap, 0).xy);

	float borderBias = max(texelSize.x, texelSize.y) * 2;

	if(projCoords.x >= 1.0 - borderBias || projCoords.x <= borderBias ||
		projCoords.y >= 1.0 - borderBias || projCoords.y <= borderBias ||
		projCoords.z >= 1.0 - borderBias || projCoords.z <= borderBias)
        return 0.0;

	float shadow = 0;
	float bias = cascadeBias[layer];
	float currentDepth = projCoords.z;

	for(int x = -1; x <= 1; ++x)
	{
		for(int y = -1; y <= 1; ++y)
		{
			float pcfDepth = texture(shadowMap, vec3(projCoords.xy + vec2(x, y) * texelSize, layer)).r;
			shadow += (currentDepth - bias > pcfDepth) ? 1.0 : 0.0;
		}
	}

	shadow /= 9.0;

	return shadow;
}