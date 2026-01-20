#frag

#define MAX_STEPS 100
#define MAX_LIGHT_STEPS 6

#pragma editable(default=0.08)
uniform float MARCH_SIZE;

vec3 sunDirection = vec3(0, 1, 0);

uniform sampler2D uNoise;

float beerLambert(float absorptionCoefficient, float distanceTraveled)
{
    return exp(-absorptionCoefficient * distanceTraveled);
}

const float ABSORPTION_COEFFICIENT = 0.9;

uniform sampler2D iChannel0;
uniform sampler2D iChannel2;

#define NOISE_METHOD 1

float noise(in vec3 x)
{
    vec3 p = floor(x);
    vec3 f = fract(x);
    f = f*f*(3.0-2.0*f);

#if NOISE_METHOD==1
    vec2 uv = (p.xy + vec2(37.0,239.0)*p.z) + f.xy;
    vec2 rg = textureLod(iChannel0,(uv+0.5)/256.0,0.0).yx;
    return mix(rg.x, rg.y, f.z)*2.0-1.0;
#endif    

#if NOISE_METHOD==2
    ivec3 q = ivec3(p);
    ivec2 uv = q.xy + ivec2(37,239)*q.z;
    vec2 rg = mix(
        mix(texelFetch(iChannel0,(uv           )&255,0),
            texelFetch(iChannel0,(uv+ivec2(1,0))&255,0),f.x),
        mix(texelFetch(iChannel0,(uv+ivec2(0,1))&255,0),
            texelFetch(iChannel0,(uv+ivec2(1,1))&255,0),f.x),
        f.y
    ).yx;
    return mix(rg.x, rg.y, f.z)*2.0-1.0;
#endif    
}

#define USE_LOD 1

float scene(in vec3 p, int oct)
{
    vec3 q = p - vec3(0.0,0.1,1.0)*iTime;
    float g = 0.5 + 0.5 * noise(q*0.3);
    
    float f;
    f  = 0.50000*noise(q); q *= 2.02;

#if USE_LOD==1
    if (oct >= 2)
#endif
    f += 0.25000*noise(q); q *= 2.23;

#if USE_LOD==1
    if (oct >= 3)
#endif
    f += 0.12500*noise(q); q *= 2.41;

#if USE_LOD==1
    if (oct >= 4)
#endif
    f += 0.06250*noise(q); q *= 2.62;

#if USE_LOD==1
    if (oct >= 5)
#endif
    f += 0.03125*noise(q);
    
    f = mix(f*0.1-0.5, f, g*g);
    return 1.5*f - 0.5 - p.y;
}

vec4 rayMarch(vec3 ro, vec3 rd, int sampleCount)
{
    float d = 0.0;
    vec4 res = vec4(0.0);

    for(int i = 0; i < sampleCount; i++)
    {
        vec3 p = ro + rd * d;

#if USE_LOD==0
        const int oct = 5;
#else
        int oct = 5 - int(log2(1.0 + iTime*0.5));
#endif

        float density = scene(p, oct);

        if (density > 0.1)
        {
            float diffuse = clamp(
                (density - scene(p + 0.3 * sunDirection, oct)) / 0.3,
                0.0, 1.0
            );

            vec3 lin = vec3(0.60,0.60,0.75) * 1.1
                     + 0.8 * vec3(1.0,0.6,0.3) * diffuse;

            vec4 color = vec4(
                mix(vec3(1.0), vec3(0.0), density),
                density
            );

            color.rgb *= lin * density;

            res.rgb += color.rgb * (1.0 - res.a);
            res.a   += color.a   * (1.0 - res.a);
        }

        d += MARCH_SIZE;
    }

    return res;
}

bool rayAABB(
    vec3 ro,
    vec3 rd,
    vec3 boxMin,
    vec3 boxMax,
    out float t0,
    out float t1
)
{
    vec3 invDir = 1.0 / max(abs(rd), vec3(1e-8)) * sign(rd);

    vec3 tMin = (boxMin - ro) * invDir;
    vec3 tMax = (boxMax - ro) * invDir;

    vec3 tEnter = min(tMin, tMax);
    vec3 tExit  = max(tMin, tMax);

    t0 = max(max(tEnter.x, tEnter.y), tEnter.z);
    t1 = min(min(tExit.x,  tExit.y),  tExit.z);

    if (t0 > t1) return false;
    if (t1 < 0.0) return false;

    return true;
}

void frag(inout vec4 color)
{
    vec2 screenUV = gl_FragCoord.xy / screenSize;
    vec2 ndc = screenUV * 2.0 - 1.0;
    ndc.x *= screenSize.x / screenSize.y;

    float tanHalfFov = tan(cameraFov * 0.5);
    vec3 rayView = normalize(vec3(ndc * tanHalfFov, -1.0));

    vec3 ro = cameraPos;
    mat3 camToWorld = transpose(mat3(view));
    vec3 rd = normalize(camToWorld * rayView);

    float tEnter, tExit;
    vec4 res = vec4(0.0);

    if (rayAABB(ro, rd, u_aabbMin, u_aabbMax, tEnter, tExit))
    {
        float hitT = max(tEnter, 0.0);
        vec3 hitPos = ro + hitT * rd;

        float marchLength = tExit - hitT;
        int sampleCount = min(int(marchLength / MARCH_SIZE), MAX_STEPS);

        res = rayMarch(hitPos, rd, sampleCount);
    }

    vec3 bgColor = texture(uMainTexture, screenUV).rgb;
    color = vec4(bgColor * (1.0 - res.a) + res.rgb, res.a);
}
