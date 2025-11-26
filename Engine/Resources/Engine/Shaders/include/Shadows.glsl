float calculateShadows(vec4 fragPos, sampler2D shadowMap)
{
	// perform perspective divide
    vec3 projCoords = fragPos.xyz / fragPos.w;
	
	vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
	
	projCoords = projCoords * 0.5 + 0.5; 
	
	float borderBias =  max(texelSize.x, texelSize.y) * 2;
	
	if(projCoords.x >= 1.0 - borderBias || projCoords.x <= borderBias ||
		projCoords.y >= 1.0 - borderBias || projCoords.y <= borderBias ||
		projCoords.z >= 1.0 - borderBias || projCoords.z <= borderBias)
        return 0.0;
	
	float shadow = 0;
	float bias = 0.005;
	float currentDepth = projCoords.z;

	for(int x = -1; x <= 1; ++x)
	{
		for(int y = -1; y <= 1; ++y)
		{
			float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
			shadow += (currentDepth - bias > pcfDepth) ? 1.0 : 0.0;
		}
	}
	
	shadow /= 9.0;
	
	return shadow;
}