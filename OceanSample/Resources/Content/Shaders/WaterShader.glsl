#vert

uniform sampler2D waterNormalSampler;

vec2 createNormalWave(vec3 aPos, float xSpeed, float ySpeed, float amp)
{
	vec2 worldPos = aPos.xy;
	worldPos *= amp;
	float t = getTime();
	worldPos.x += t * xSpeed;
	worldPos.y += t * ySpeed;
	return texture(waterNormalSampler, worldPos).xy;
}

void vert(inout vec3 aPos, inout vec3 aNorm)
{
	// vec2 normalWave = vec2(0.0);
	// normalWave += createNormalWave(aPos, 0.01, 0.001, 3.0);
	// normalWave += createNormalWave(aPos, 0.001, -0.01, 3.5);
	// normalWave += createNormalWave(aPos, 0.001, -0.001, 1.0);

	// //float z = sqrt(1 - normalWave.x * normalWave.x - normalWave.y * normalWave.y);

	// aNorm = vec3(normalWave.x, 1, normalWave.y);
}

#frag

uniform sampler2D waterNormalSampler;

vec2 createNormalWave(vec3 aPos, float xSpeed, float ySpeed, float amp)
{
	vec2 worldPos = aPos.xz;
	worldPos *= amp;
	float t = getTime();
	worldPos.x += t * xSpeed;
	worldPos.y += t * ySpeed;
	return texture(waterNormalSampler, worldPos).rg;
}

void frag(inout vec3 albedo, inout vec3 normal, inout float metallic, inout float roughness, inout float ao)
{
	float pixelDistance = getCameraPosition().y - getPixelPosition().y;
	pixelDistance /= 100.0;
	pixelDistance = pow(pixelDistance, 0.2); 

	// do feresnel maybe

	pixelDistance = clamp(pixelDistance, 0.0, 1.0);
	vec3 colorA = vec3(0.023497, 0.451692, 1.0);
    vec3 colorB = vec3(0.0, 0.05938, 0.135417);

	//color = vec3(1.0,0.0,0.0);
	//color = mix(colorA, colorB, pixelDistance) * color;
	albedo = mix(colorA, colorB, pixelDistance);
	// albedo = colorB;
	// albedo = vec3(clamp(pixelDistance,0,1),0,0);
	roughness = .8;
	metallic = 0.0;
	// albedo = vec3(1,0,0);

	vec3 worldPos = getPixelPosition() / 100;

	// Set normal
	vec2 normalWave = vec2(0.0);
	normalWave += createNormalWave(worldPos, 0.1, 0.01, 3.0);
	normalWave += createNormalWave(worldPos, 0.01, -0.1, 3.5);
	normalWave += createNormalWave(worldPos, 0.01, -0.01, 1.0);

	float z = sqrt(1 - normalWave.x * normalWave.x - normalWave.y * normalWave.y);

	normal = vec3(normalWave.x, 1, normalWave.y);
}