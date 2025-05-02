#frag

#define MAX_STEPS 200
#define MAX_LIGHT_STEPS 6
const float MARCH_SIZE = 0.02;
const float LIGHT_MARCH_SIZE = 0.06;
const float ABSORPTION_COEFFICIENT = .9;
const float densityScale = .11;

vec3 sunDirection = vec3(0, 1, 0);

uniform sampler3D test;

// float noise(vec3 x ) {
//   vec3 p = floor(x);
//   vec3 f = fract(x);
//   f = f*f*(3.0-2.0*f);

//   vec2 nuv = (p.xy+vec2(37.0,239.0)*p.z) + f.xy;
//   vec2 tex = textureLod(uNoise,(nuv+0.5)/256.0, 0.0).yx;

//   return mix( tex.x, tex.y, f.z ) * 2.0 - 1.0;
// }

// float fbm(vec3 p) {
//   vec3 q = p + iTime * 0.5 * vec3(1.0, -0.2, -1.0);
//   float g = noise(q);

//   float f = 0.0;
//   float scale = 0.5;
//   float factor = 2.02;

//   for (int i = 0; i < 6; i++) {
//       f += scale * noise(q);
//       q *= factor;
//       factor += 0.21;
//       scale *= 0.5;
//   }

//   return f;
// }

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

float sdBox( vec3 p, vec3 b )
{
  vec3 q = abs(p) - b;
  return length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0);
}

float sceneSDF(vec3 pos)
{
    float tex = texture(test, pos).r;
    
    float distance = 1.0 - sdBox(pos, vec3(1,1,1));

    return distance * tex;

    // vec3 q = pos - vec3(1.0,0.2,1.0)*iTime * .4;
    // float f = fbm(q);

    // return -distance + f;
}

float beerLambert(float absorptionCoefficient, float distanceTraveled)
{
    return exp(-absorptionCoefficient * distanceTraveled);
}

float lightMarch(vec3 p0)
{
    vec3 rd = normalize(sunDirection);
    float totalDensity = 0.0;
    float d = LIGHT_MARCH_SIZE;
    for(int i=0; i<MAX_LIGHT_STEPS; i++)
    {
        vec3 p = p0 + rd * d;
        float density = sceneSDF(p);
        totalDensity += density;// * marchSize;
        d += LIGHT_MARCH_SIZE;
    } 

    float transmittance = beerLambert(ABSORPTION_COEFFICIENT, totalDensity);
    return transmittance;
}

float rayMarch(vec3 ro, vec3 rd)
{
    float d = 0.;
    vec4 res = vec4(0.0);
    float transmittance = 1.;
    float lightAbsorb = 2.02;
    float density = 0.;
    float darknessThreshold = 0.1;

    float finalLight = 0.0;

    for(int i=0; i<MAX_STEPS; i++)
    {
        vec3 p = ro + rd * d;
        float sampledDensity = sceneSDF(p);
        if(sampledDensity > 0.0)
        {
            float lightTransmission = lightMarch(p);
            float shadow = darknessThreshold + lightTransmission * (1.0 -darknessThreshold);
            density += sampledDensity * densityScale;

            // for this point p 
            // shadow is how much has reached this point from the sun
            // density is how much light will pass on from this point onward 
            // (this is odd since I thought sampled density makes more sense here, density is already embodied in transmittance)
            // transmittance is how much light will reach the camera
            finalLight += density * transmittance * shadow; 
            transmittance *= exp(-density*lightAbsorb);
        }
        d += MARCH_SIZE;
    } 
    return finalLight;
}

mat3 lookAt(vec3 ro, vec3 target) {
    vec3 f = normalize(target - ro);
    vec3 r = normalize(cross(vec3(0,1,0), f));
    vec3 u = cross(f, r);
    return mat3(r, u, f);
}

void frag(inout vec3 color)
{
    vec2 xy = uv - .5;
    xy *= vec2(1, -1); // hack
    vec3 ro = vec3(0.0, 0.0, 5.0);
    vec3 rd = normalize(vec3(xy, -1));

    float res = rayMarch(ro, rd);
    color = vec3(res);
}