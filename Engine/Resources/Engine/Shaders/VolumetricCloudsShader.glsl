#frag

#define MAX_STEPS 100
#define MAX_LIGHT_STEPS 6

#pragma editable(default=0.08)
uniform float MARCH_SIZE;

vec3 sunDirection = vec3(0, 1, 0);

uniform sampler2D uNoise;
uniform sampler2D iChannel0;

// noise
// Volume raycasting by XT95
// https://www.shadertoy.com/view/lss3zr
mat3 m = mat3( 0.00,  0.80,  0.60,
              -0.80,  0.36, -0.48,
              -0.60, -0.48,  0.64 );
float hash( float n )
{
    return fract(sin(n)*43758.5453);
}

float noise( in vec3 x )
{
    vec3 p = floor(x);
    vec3 f = fract(x);
	f = f*f*(3.0-2.0*f);
	vec2 uv = (p.xy+vec2(37.0,239.0)*p.z) + f.xy;
    vec2 rg = textureLod(iChannel0,(uv+0.5)/128.0, 0.0).yx;
	return mix( rg.x, rg.y, f.z )*2.0-1.0;
}

float noise_orig( in vec3 x )
{
    vec3 p = floor(x);
    vec3 f = fract(x);

    f = f*f*(3.0-2.0*f);

    float n = p.x + p.y*57.0 + 113.0*p.z;

    float res = mix(mix(mix( hash(n+  0.0), hash(n+  1.0),f.x),
                        mix( hash(n+ 57.0), hash(n+ 58.0),f.x),f.y),
                    mix(mix( hash(n+113.0), hash(n+114.0),f.x),
                        mix( hash(n+170.0), hash(n+171.0),f.x),f.y),f.z);
    return res;
}

float fbm( vec3 p )
{
    float f;
    f  = 0.5000*noise( p ); p = m*p*2.02;
    f += 0.2500*noise( p ); p = m*p*2.03;
    f += 0.12500*noise( p ); p = m*p*2.01;
    f += 0.06250*noise( p );
    return f;
}

float sdSphere(vec3 p, float radius) 
{
    return length(p) - radius;
}

float sceneSDF(vec3 pos)
{
    float distance = sdSphere(pos, 2);

    vec3 q = pos - vec3(1.0,0.2,1.0)*iTime;
    float f = fbm(q);

    return -distance + f;
}

float beerLambert(float absorptionCoefficient, float distanceTraveled)
{
    return exp(-absorptionCoefficient * distanceTraveled);
}

const float ABSORPTION_COEFFICIENT = 0.9;

float lightMarch(vec3 p0)
{
    vec3 rd = normalize(sunDirection);
    float marchSize = 0.03;
    float totalDensity = 0.0;
    float d = marchSize;
    for(int i=0; i<MAX_LIGHT_STEPS; i++)
    {
        vec3 p = p0 + rd * d;
        float density = sceneSDF(p);
        totalDensity += density;// * marchSize;
        d += marchSize;
    } 

    float transmittance = beerLambert(ABSORPTION_COEFFICIENT, totalDensity);
    return transmittance;
}

vec4 rayMarch(vec3 ro, vec3 rd, int sampleCount)
{
    float d = fract(sin(dot(ro.xy + rd.xy, vec2(12.9898, 78.233))) * 43758.5453) * MARCH_SIZE;
    vec4 res = vec4(0.0);

    float totalTransmittance = 1.0;
    float lightEnergy = 0.0;

    for(int i=0; i<sampleCount; i++)
    {
        vec3 p = ro + rd * d;
        float density = sceneSDF(p);
        if(density > 0.0)
        {
            // float transmittance = lightMarch(p);
            // totalTransmittance *= transmittance;
            // float luminance = density;
            // lightEnergy += totalTransmittance * luminance;

            // Directional derivative for fast diffuse lighting
            float diffuse = clamp((density - sceneSDF(p + .3 * sunDirection)) / .3, .0, 1.);
            vec3 lin = vec3(0.60,0.60,0.75) * 1.1 + 0.8 * vec3(1.0,0.6,0.3) * diffuse;
            vec4 color = vec4(mix(vec3(1.0,1.0,1.0), vec3(0.0, 0.0, 0.0), density), density );
            color.rgb *= lin * density;
            res += color * (1.0 - res.a);
        }
        d += MARCH_SIZE;

    } 
    return res;
}

bool rayAABB(
    vec3 ro,        // ray origin
    vec3 rd,        // ray direction (can be normalized or not)
    vec3 boxMin,    // AABB min
    vec3 boxMax,    // AABB max
    out float t0,   // entry distance
    out float t1    // exit distance
)
{
    vec3 invDir = 1.0 / max(abs(rd), vec3(1e-8)) * sign(rd);

    vec3 tMin = (boxMin - ro) * invDir;
    vec3 tMax = (boxMax - ro) * invDir;

    vec3 tEnter = min(tMin, tMax);
    vec3 tExit  = max(tMin, tMax);

    t0 = max(max(tEnter.x, tEnter.y), tEnter.z);
    t1 = min(min(tExit.x,  tExit.y),  tExit.z);

    // no intersection
    if (t0 > t1)
        return false;

    // box is completely behind the ray
    if (t1 < 0.0)
        return false;

    return true;
}


void frag(inout vec4 color)
{
    vec2 screenPos = gl_FragCoord.xy; // x in range [0.5 , width − 0.5]
    vec2 screenUV = screenPos / screenSize; // (0,1)
    vec2 ndc = screenUV * 2.0 - 1.0;   // [-1, 1]
    ndc.x *= screenSize.x / screenSize.y; // aspect correction

    float tanHalfFov = tan(cameraFov * 0.5);
    vec3 rayView = normalize(vec3(ndc.x * tanHalfFov, ndc.y * tanHalfFov, -1.0));

    vec3 ro = cameraPos;
    mat3 camToWorld = transpose(mat3(view)); //from view-space to world-space
    vec3 rd = normalize(camToWorld * rayView);

    float tEnter, tExit;
    vec4 res = vec4(0.0);

    if (rayAABB(ro, rd, u_aabbMin, u_aabbMax, tEnter, tExit))
    {
        float hitT = max(tEnter, 0.0); // clamp if origin is inside box
        vec3 hitPos = ro + hitT * rd;

        float marchLength = tExit - hitT;
        int sampleCount = min(int(marchLength / MARCH_SIZE), MAX_STEPS);

        res = rayMarch(hitPos, rd, sampleCount);
    }

    vec3 volumeColor = vec3(1.0);

    vec3 bgColor = texture(uMainTexture, screenUV).rgb;

    color = vec4(bgColor * (1.0 - res.a) + res.rgb, res.a);;
}