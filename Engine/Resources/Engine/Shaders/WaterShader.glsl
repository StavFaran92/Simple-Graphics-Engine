#vert

void vert(inout vec3 aPos, inout vec3 aNorm)
{
}

#frag

#pragma editable
uniform PBR_Sampler uWaterNormalSampler;

#pragma editable(default=(0.1, 0.01))
uniform vec2 uWave1Speed;

#pragma editable(default=(0.01, -0.1))
uniform vec2 uWave2Speed;

#pragma editable(default=(0.01, -0.01))
uniform vec2 uWave3Speed;

#pragma editable(default=(3.0))
uniform float uWave1Amp;

#pragma editable(default=(3.5))
uniform float uWave2Amp;

#pragma editable(default=(1.0))
uniform float uWave3Amp;

#pragma editable(default=(0.023497, 0.451692, 1.0))
uniform vec3 uColorA;

#pragma editable(default=(0.0, 0.05938, 0.135417))
uniform vec3 uColorB;

vec2 createNormalWave(vec3 aPos, float xSpeed, float ySpeed, float amp)
{
	vec2 worldPos = aPos.xz;
	worldPos *= amp;
	float t = getTime();
	worldPos.x += t * xSpeed;
	worldPos.y += t * ySpeed;
	return texture(uWaterNormalSampler.texture, worldPos).rg;
}

void frag(inout vec3 albedo, inout vec3 normal, inout float metallic, inout float roughness, inout float ao)
{
	// to get cool ocean effect set R=0.8, remove normal division and remap, and use 1 as Z

	// Get pixel distance
	float pixelDistance = getCameraPosition().y - getPixelPosition().y;
	pixelDistance /= 100.0;
	pixelDistance = pow(pixelDistance, 0.2); 
	pixelDistance = clamp(pixelDistance, 0.0, 1.0);
	vec3 colorA = uColorA;
    vec3 colorB = uColorB;
	// TODO do feresnel maybe
	albedo = mix(colorA, colorB, pixelDistance);

	roughness = 0.0;
	metallic = 0.0;

	// Set normal
	vec3 worldPos = getPixelPosition() / 100.0;
	vec2 normalWave = vec2(0.0);
	normalWave += createNormalWave(worldPos, uWave1Speed.x, uWave1Speed.y, uWave1Amp);
	normalWave += createNormalWave(worldPos, uWave2Speed.x, uWave2Speed.y, uWave2Amp);
	normalWave += createNormalWave(worldPos, uWave3Speed.x, uWave3Speed.y, uWave3Amp);
	normalWave /= 3.0; // Divide by number of sampled waves (each wave sample is in range [0,1])
	normalWave = normalWave * 2.0 - 1.0; // remap from [0,1] -> [-1,1]

	float z = sqrt(1 - normalWave.x * normalWave.x - normalWave.y * normalWave.y);
	normal = vec3(normalWave.x, z, normalWave.y);
}