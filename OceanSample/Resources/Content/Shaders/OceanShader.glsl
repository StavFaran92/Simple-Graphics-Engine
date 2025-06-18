#vert

uniform float amplitude;
uniform vec2 waveDirection;
uniform float waveLength;
uniform float waveSpeed;
uniform float steepness;

void vert(inout vec3 aPos, inout vec3 aNorm)
{
	float w = PI2 / waveLength;
	float pSpeed = 2 * waveSpeed / waveLength;
	float q = steepness / (w * amplitude);
	
	vec2 dir = normalize(waveDirection);

	float a = dot(vec2(aPos.x, aPos.y),dir) * w + getTime() * pSpeed;

	aPos.x = aPos.x + q * amplitude * dir.x * cos(a);
	aPos.y = aPos.y + q * amplitude * dir.y * cos(a);
	aPos.z = - amplitude * sin(a);
	
	aNorm.x = - dir.x * w * amplitude * cos(a);
	aNorm.y = - dir.y * w * amplitude * cos(a);
	aNorm.z = 1 - q * w * amplitude * sin(a);
}

#frag

void frag(inout vec3 color)
{
	float pixelDistance = getCameraPosition().y - getPixelPosition().y;
	pixelDistance /= 100.0;
	pixelDistance = pow(pixelDistance, 0.2); 

	// do feresnel maybe

	pixelDistance = clamp(pixelDistance, 0.0, 1.0);
	vec3 colorA = vec3(0.023497, 0.451692, 1.0);
    vec3 colorB = vec3(0.0, 0.05938, 0.135417);

	color = colorA * pixelDistance * color + colorB;
}