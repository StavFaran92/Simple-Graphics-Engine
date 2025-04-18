#frag

const int MAX_MARCHING_STEPS = 255;
const int MAX_SDF_SPHERE_STEPS = 15;
const int MAX_VOLUME_MARCH_STEPS = 50;
const float MIN_DIST = 0.0;
const float MAX_DIST = 100.0;
const float EPSILON = 0.0001;
const float ABSORPTION_COEFFICIENT = 0.5;
const float LIGHT_ATTENUATION_FACTOR = 2.0;

struct Light
{
    vec3 pos;
    vec3 color;
};

// Taken from https://iquilezles.org/www/articles/distfunctions/distfunctions.htm
float sdSmoothUnion( float d1, float d2, float k ) 
{
    float h = clamp( 0.5 + 0.5*(d2-d1)/k, 0.0, 1.0 );
    return mix( d2, d1, h ) - k*h*(1.0-h); 
}

// --------------------------------------------//
//               Noise Functions
// --------------------------------------------//
// Taken from Inigo Quilez's Rainforest ShaderToy:
// https://www.shadertoy.com/view/4ttSWf
float hash1( float n )
{
    return fract( n*17.0*fract( n*0.3183099 ) );
}

// Taken from Inigo Quilez's Rainforest ShaderToy:
// https://www.shadertoy.com/view/4ttSWf
float noise( in vec3 x )
{
    vec3 p = floor(x);
    vec3 w = fract(x);
    
    vec3 u = w*w*w*(w*(w*6.0-15.0)+10.0);
    
    float n = p.x + 317.0*p.y + 157.0*p.z;
    
    float a = hash1(n+0.0);
    float b = hash1(n+1.0);
    float c = hash1(n+317.0);
    float d = hash1(n+318.0);
    float e = hash1(n+157.0);
	float f = hash1(n+158.0);
    float g = hash1(n+474.0);
    float h = hash1(n+475.0);

    float k0 =   a;
    float k1 =   b - a;
    float k2 =   c - a;
    float k3 =   e - a;
    float k4 =   a - b - c + d;
    float k5 =   a - c - e + g;
    float k6 =   a - b - e + f;
    float k7 = - a + b + c - d + e - f - g + h;

    return -1.0+2.0*(k0 + k1*u.x + k2*u.y + k3*u.z + k4*u.x*u.y + k5*u.y*u.z + k6*u.z*u.x + k7*u.x*u.y*u.z);
}

const mat3 m3  = mat3( 0.00,  0.80,  0.60,
                      -0.80,  0.36, -0.48,
                      -0.60, -0.48,  0.64 );

// Taken from Inigo Quilez's Rainforest ShaderToy:
// https://www.shadertoy.com/view/4ttSWf
float fbm_4( in vec3 x )
{
    float f = 2.0;
    float s = 0.5;
    float a = 0.0;
    float b = 0.5;
    for( int i=0; i<4; i++ )
    {
        float n = noise(x);
        a += b*n;
        b *= s;
        x = f*m3*x;
    }
	return a;
}

float sphereSDF(vec3 samplePoint, vec3 origin, float r) {
    return length(samplePoint - origin) - r;
}

float planeSDF(vec3 samplePoint) {
    return samplePoint.y;
}

float queryVolumetricDistanceField(vec3 samplePoint) {
	
    float sdfValue = sphereSDF(samplePoint, vec3(0, 1, 0), 1);
	sdfValue = sdSmoothUnion(sdfValue, sphereSDF(samplePoint, vec3(1, 2, 0), .7), .3);

	vec3 fbmCoord = (samplePoint + 2.0 * vec3(iTime, 0.0, iTime)) / 1.5f;
	sdfValue += 7.0 * fbm_4(fbmCoord / 3.2);
	
	sdfValue = sdSmoothUnion(sdfValue, planeSDF(samplePoint), .3);

    return sdfValue;
}

float rayMarch(vec3 ro, vec3 rd)
{
    float d = 0.;
    for(int i=0; i<MAX_MARCHING_STEPS; i++)
    {
        float ds = queryVolumetricDistanceField(ro + rd * d);
        d += ds;
        if(ds < EPSILON || d > MAX_DIST) break;
    } 

    return d;
}

vec3 getNormal(vec3 p)
{
    float dist = queryVolumetricDistanceField(p);
    vec2 e = vec2(.01, 0);

    vec3 n = vec3(
        dist - queryVolumetricDistanceField(p-e.xyy),
        dist - queryVolumetricDistanceField(p-e.yxy),
        dist - queryVolumetricDistanceField(p-e.yyx)
    );

    return normalize(n);
}



// vec3 getLight(vec3 p)
// {
//     sunLight.pos = vec3(cos(iTime) * 1, 0, sin(iTime)* 1);
//     sunLight.color = vec3(1,0,0);
//     vec3 ld = normalize(sunLight.pos - p);
//     vec3 n = getNormal(p);

//     float diff = clamp(dot(n, ld), 0., 1.);

//     float distToLight = rayMarch(p + n * 0.1, ld);
//     if(distToLight < length(sunLight.pos - p))
//     {
//         diff *= .1;
//     }

    
//     return sunLight.color * diff;
// }

float intersectVolumetric(vec3 ro, vec3 rd)
{
    // Precision isn't super important, just want a decent starting point before 
    // ray marching with fixed steps
	float precis = 0.5; 
    float t = 0.0f;
    for(int i=0; i<MAX_SDF_SPHERE_STEPS; i++ )
    {
	    float result = queryVolumetricDistanceField( ro+rd*t);
        if( result < (precis) || t>MAX_DIST ) break;
        t += result;
    }
    return ( t>=MAX_DIST ) ? -1.0 : t;
}

float beerLambert(float absorptionCoefficient, float distanceTraveled)
{
    return exp(-absorptionCoefficient * distanceTraveled);
}

float getLightAttenuation(float distanceToLight)
{
    return 1.0 / pow(distanceToLight, LIGHT_ATTENUATION_FACTOR);
}

uniform vec2 a; 

vec3 GetAmbientLight()
{
	return 1.2 * vec3(0.03, 0.018, 0.018);
}

mat3 lookAt(vec3 ro, vec3 target) {
    vec3 f = normalize(target - ro);
    vec3 r = normalize(cross(vec3(0,1,0), f));
    vec3 u = cross(f, r);
    return mat3(r, u, f);
}

vec3 GammaCorrect(vec3 color) 
{
    return pow(color, vec3(1.0/2.2));
}

void frag(inout vec3 color)
{
    vec2 xy = uv - .5;
    xy *= vec2(1, -1); // hack
    vec3 ro = vec3(0.0, 15.0, -15.0);
    vec3 rd = normalize(vec3(xy, 1));

    vec3 target = vec3(0.0);
    rd = lookAt(ro, target) * rd;

    Light sunLight;
    sunLight.pos = vec3(cos(iTime) * 1, 10, sin(iTime)* 1);
    sunLight.color = vec3(1,1,1);

    float volumeDepth = intersectVolumetric(ro, rd);

    vec3 volumetricColor = vec3(0.0f);
    if(volumeDepth > 0.0)
    {
        float opaqueVisiblity = 1.0f;
        const float marchSize = 0.1f;
        const vec3 volumeAlbedo = vec3(0.8);
        for(int i=0; i<MAX_VOLUME_MARCH_STEPS; i++)
        {
            volumeDepth += marchSize;
            vec3 pos = ro + rd * volumeDepth;
            bool isInVolume = queryVolumetricDistanceField(pos) < 0.0;
            if(isInVolume)
            {
                //color = vec3(1,0,0);
                float previousOpaqueVisiblity = opaqueVisiblity;
                opaqueVisiblity *= beerLambert(ABSORPTION_COEFFICIENT, marchSize);
                float absorptionFromMarch = previousOpaqueVisiblity - opaqueVisiblity;

                float distanceToLight = length((sunLight.pos - pos));
                volumetricColor += absorptionFromMarch * volumeAlbedo * getLightAttenuation(distanceToLight) * sunLight.color;
            }
            
        }
    }

    color = volumetricColor + GetAmbientLight();
    color = vec3(GammaCorrect(clamp(color, 0.0, 1.0)));
}