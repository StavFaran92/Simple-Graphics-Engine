#frag

const int MAX_MARCHING_STEPS = 255;
const float MIN_DIST = 0.0;
const float MAX_DIST = 100.0;
const float EPSILON = 0.0001;

float sphereSDF(vec3 samplePoint) {
    vec3 spherePos = vec3(0, 1, 6);
    float sphereR = 1; 
    return length(samplePoint - spherePos) - sphereR;
}

float planeSDF(vec3 samplePoint) {
    return samplePoint.y;
}

float sceneSDF(vec3 samplePoint) {
    float distToSphere = sphereSDF(samplePoint);
    float distToPlane = planeSDF(samplePoint);

    float finalDistance = min(distToSphere, distToPlane);
    return finalDistance;
}

float rayMarch(vec3 ro, vec3 rd)
{
    float d = 0.;
    for(int i=0; i<MAX_MARCHING_STEPS; i++)
    {
        float ds = sceneSDF(ro + rd * d);
        d += ds;
        if(ds < EPSILON || d > MAX_DIST) break;
    } 

    return d;
}

vec3 getNormal(vec3 p)
{
    float dist = sceneSDF(p);
    vec2 e = vec2(.01, 0);

    vec3 n = vec3(
        dist - sceneSDF(p-e.xyy),
        dist - sceneSDF(p-e.yxy),
        dist - sceneSDF(p-e.yyx)
    );

    return normalize(n);
}

vec3 getLight(vec3 p)
{
    vec3 lightPos = vec3(cos(iTime) * 10, 5, sin(iTime)* 10);
    vec3 lightColor = vec3(1,0,0);
    vec3 ld = normalize(lightPos - p);
    vec3 n = getNormal(p);

    float diff = clamp(dot(n, ld), 0., 1.);

    float distToLight = rayMarch(p + n * 0.1, ld);
    if(distToLight < length(lightPos - p))
    {
        diff *= .1;
    }

    
    return lightColor * diff;
}

uniform vec2 a; 

void frag(inout vec3 color)
{
    vec2 xy = uv - .5;
    xy *= vec2(1, -1); // hack
    vec3 ro = vec3(0.0, 1.0, 0.0);
    vec3 rd = normalize(vec3(xy, 1));
    float dist = rayMarch(ro, rd);
    
    if (dist > MAX_DIST - EPSILON) {
        // Didn't hit anything
        color = vec3(0.0, 0.0, 0.0);
		return;
    }
    
    vec3 p = ro + rd * dist;
    color = getLight(p);
}