#vert

#version 420 core
                                                                                    
layout (location = 0) in vec3 aPos;  
layout (location = 2) in vec2 aTexCoords;                                        

out vec2 texCoords;
out mat3 TBN;

uniform int width;
uniform int height;
                                                            
void main()                                                                         
{
    texCoords = aTexCoords;
    vec3 pos = aPos.xyz;
    pos *= vec3(width, 0, height);
    gl_Position = vec4(pos, 1.0);                                       
}

#tesc

#version 420 core

// Number of vertices in a patch
layout (vertices=4) out;

uniform mat4 model;
uniform mat4 view;

in vec2 texCoords[];

out vec2 outTexCoords[];

const int MIN_TESS_LEVEL = 4;
const int MAX_TESS_LEVEL = 64;
const float MIN_DISTANCE = 20;
const float MAX_DISTANCE = 200;

void main()
{
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    outTexCoords[gl_InvocationID] = texCoords[gl_InvocationID];

    //If we are in Vertex 0 in the patch
    if(gl_InvocationID == 0)
    {
        // transform each vertex into eye space
        vec4 eyeSpacePos00 = view * model * gl_in[0].gl_Position;
        vec4 eyeSpacePos01 = view * model * gl_in[1].gl_Position;
        vec4 eyeSpacePos10 = view * model * gl_in[2].gl_Position;
        vec4 eyeSpacePos11 = view * model * gl_in[3].gl_Position;

        // "distance" from camera scaled between 0 and 1
        float distance00 = clamp((abs(eyeSpacePos00.z)-MIN_DISTANCE) / (MAX_DISTANCE-MIN_DISTANCE), 0.0, 1.0);
        float distance01 = clamp((abs(eyeSpacePos01.z)-MIN_DISTANCE) / (MAX_DISTANCE-MIN_DISTANCE), 0.0, 1.0);
        float distance10 = clamp((abs(eyeSpacePos10.z)-MIN_DISTANCE) / (MAX_DISTANCE-MIN_DISTANCE), 0.0, 1.0);
        float distance11 = clamp((abs(eyeSpacePos11.z)-MIN_DISTANCE) / (MAX_DISTANCE-MIN_DISTANCE), 0.0, 1.0);

        // interpolate edge tessellation level based on closer vertex
        float tessLevel0 = mix( MAX_TESS_LEVEL, MIN_TESS_LEVEL, min(distance10, distance00) );
        float tessLevel1 = mix( MAX_TESS_LEVEL, MIN_TESS_LEVEL, min(distance00, distance01) );
        float tessLevel2 = mix( MAX_TESS_LEVEL, MIN_TESS_LEVEL, min(distance01, distance11) );
        float tessLevel3 = mix( MAX_TESS_LEVEL, MIN_TESS_LEVEL, min(distance11, distance10) );

        // set the corresponding outer edge tessellation levels
        gl_TessLevelOuter[0] = tessLevel0;
        gl_TessLevelOuter[1] = tessLevel1;
        gl_TessLevelOuter[2] = tessLevel2;
        gl_TessLevelOuter[3] = tessLevel3;

        // set the inner tessellation levels to the max of the two parallel edges
        gl_TessLevelInner[0] = max(tessLevel1, tessLevel3);
        gl_TessLevelInner[1] = max(tessLevel0, tessLevel2);
    }
}

#tese

#version 420 core

layout (quads, fractional_odd_spacing, ccw) in;

uniform sampler2D heightMap;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float scale;

in vec2 outTexCoords[];

out vec2 texCoord;

// Send to fragment shader for coloring
out float height;
out vec3 fragPos;
out vec3 fragNormal;
out vec3 tangent;
out vec3 bitangent;

void main()
{
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;

    // Interpolate UVs across the patch
    vec2 t00 = outTexCoords[0];
    vec2 t01 = outTexCoords[1];
    vec2 t10 = outTexCoords[2];
    vec2 t11 = outTexCoords[3];

    vec2 t0 = mix(t00, t01, u);
    vec2 t1 = mix(t10, t11, u);
    vec2 texCoordLocal = mix(t0, t1, v);

    // Clamp to valid texel range
    vec2 texSize = textureSize(heightMap, 0);
    vec2 texel   = 1.0 / texSize;
    vec2 halfTexel = 0.5 * texel;
    texCoord = clamp(texCoordLocal, halfTexel, 1.0 - halfTexel);

    // Sample height and neighbors (central differences)
    float h  = texture(heightMap, texCoord).r;
    float hL = texture(heightMap, texCoord - vec2(texel.x, 0)).r;
    float hR = texture(heightMap, texCoord + vec2(texel.x, 0)).r;
    float hD = texture(heightMap, texCoord - vec2(0, texel.y)).r;
    float hU = texture(heightMap, texCoord + vec2(0, texel.y)).r;

    // Height derivatives
    float dhdx = (hR - hL) * 0.5;
    float dhdz = (hU - hD) * 0.5;

    // Patch position interpolation (object space)
    vec4 p00 = gl_in[0].gl_Position;
    vec4 p01 = gl_in[1].gl_Position;
    vec4 p10 = gl_in[2].gl_Position;
    vec4 p11 = gl_in[3].gl_Position;

    vec4 p0 = mix(p00, p01, u);
    vec4 p1 = mix(p10, p11, u);
    vec4 p  = mix(p0, p1, v);

    // Displace vertex
    p.y += h * scale;

    // Build correct TBN for height surface y = f(x, z)
    vec3 T = vec3(1.0, dhdx * scale, 0.0); // dP/dx
    vec3 B = vec3(0.0, dhdz * scale, 1.0); // dP/dz

    vec3 N = normalize(cross(B, T));

    // Orthonormalize
    T = normalize(T - N * dot(N, T));
    B = normalize(cross(N, T));

    // Transform to world space
    mat3 normalMatrix = mat3(transpose(inverse(model)));

    fragNormal = normalize(normalMatrix * N);
    tangent    = normalize(normalMatrix * T);
    bitangent  = normalize(normalMatrix * B);

    fragPos = (model * p).xyz;
    gl_Position = projection * view * model * p;
}

#frag

#version 420 core

#include ../../../../Engine/Resources/Engine/Shaders/include/defines.glsl
#include ../../../../Engine/Resources/Engine/Shaders/include/structs.glsl
#include ../../../../Engine/Resources/Engine/Shaders/include/uniforms.glsl
#include ../../../../Engine/Resources/Engine/Shaders/include/functions.glsl
#include ../../../../Engine/Resources/Engine/Shaders/include/PBR.glsl
#include ../../../../Engine/Resources/Engine/Shaders/include/Shadows.glsl

uniform float scale;

uniform vec3 cameraPos;
uniform samplerCube gIrradianceMap;
uniform samplerCube gPrefilterEnvMap;
uniform sampler2D gBRDFIntegrationLUT;
uniform sampler2D gShadowMap;
layout(binding = 5) uniform sampler2D noise;

uniform int layerCount;

struct TerrainLayer
{
    PBR_Sampler texturePack0;
    PBR_Sampler texturePack1;
    vec2 uv;
};

layout(binding = 6) uniform sampler2D terrainLayerMask[3];

uniform TerrainLayer terrainLayers[3];

#pragma editable (default=(100.0, 100.0))
uniform vec2 globalUV;

in float height;
in vec2 texCoord;
in vec3 fragPos;
in vec3 fragNormal;
in vec3 tangent;
in vec3 bitangent;

out vec4 FragColor;

float sum( vec4 v ) { return v.x+v.y+v.z+v.w; }

vec4 textureNoTile(sampler2D tex, in vec2 uv)
{
    float k = texture( noise, 0.005 * uv ).x; // cheap (cache friendly) lookup
    
    vec2 duvdx = dFdx( uv );
    vec2 duvdy = dFdy( uv );
    
    float l = k*8.0;
    float f = fract(l);
    
    float ia = floor(l);
    float ib = ia + 1.0;
    
    vec2 offa = sin(vec2(3.0,7.0)*ia); // can replace with any other hash
    vec2 offb = sin(vec2(3.0,7.0)*ib); // can replace with any other hash

    vec4 cola = textureGrad( tex, uv + offa, duvdx, duvdy );
    vec4 colb = textureGrad( tex, uv + offb, duvdx, duvdy );
    
    return mix( cola, colb, smoothstep(0.2,0.8,f-0.1*sum(cola-colb)) );
}

void sampleTerrainPBR(
    in mat3 TBN,
    in vec3 normalIn,
	in vec2 uv,

    in TerrainLayer terrainLayer,

    out vec3 outNormal,
    out vec3 outAlbedo,
    out vec3 outMRA
)
{
    vec4 texPack0 = textureNoTile(terrainLayer.texturePack0.texture, uv);
    vec4 texPack1 = textureNoTile(terrainLayer.texturePack1.texture, uv);
    vec3 albedo = texPack0.rgb;
    float nx = texPack0.a;
    float ny = texPack1.a;
    float nz = sqrt(1.0 - nx*nx - ny*ny);
    vec3 normal = vec3(nx, ny, nz);
    float metallic = texPack1.r;
    float roughness = texPack1.g;
    float ao = texPack1.b;

    outNormal = (TBN * (normal * 2.0 - 1.0));
                                       

    vec3 albedoTex = albedo;
    outAlbedo = albedoTex;// * color;
    outAlbedo = pow(outAlbedo, vec3(2.2)); // Gamma correction

    float metallicTex = metallic;
    outMRA.r = metallicTex;// * metallicFactor;

    float roughnessTex = roughness;
    outMRA.g = roughnessTex;// * roughnessFactor;

    float aoTex = ao;
    outMRA.b = aoTex;
}

void main()
{
    mat3 TBN = mat3(tangent, bitangent, fragNormal);

    vec3 totalAlbedo = vec3(0.0);
    vec3 totalNormal = vec3(0.0);
    vec3 totalMRA    = vec3(0.0);
    float opacityLeft = 1.0;
    int layerIndex = layerCount;
    while(layerIndex > 0 && opacityLeft > 0.0)
    {
        float opacity;
        // If base layer use left opacity (base is always full)
        if(layerIndex == 1)
        {
            opacity = opacityLeft;
        }
        else
        {
            float sampledOpacity = texture(terrainLayerMask[layerIndex-1], texCoord).r;
            opacity = min(sampledOpacity, opacityLeft);  
        }    

        vec3 normal;
        vec3 albedo;
        vec3 MRA;
        sampleTerrainPBR(
            // Input
            TBN,
            fragNormal,
            texCoord * globalUV,
            
            terrainLayers[layerIndex-1],

            // Output
            normal,
            albedo,
            MRA
        );

        totalAlbedo += opacity * albedo;
        totalNormal += opacity * normal;
        totalMRA += opacity * MRA;

        opacityLeft -= opacity;
        layerIndex--;
    }


    vec4 fragPosInLightSpace = lightSpaceMatrix * vec4(fragPos, 1.f);
    float shadow = calculateShadows(fragPosInLightSpace, gShadowMap);

    vec3 color = calculatePBR(
                totalAlbedo,
                totalNormal,
                totalMRA.r,
                totalMRA.g,
                totalMRA.b,
                cameraPos,
                fragPos,
                shadow,
                gPrefilterEnvMap,
                gIrradianceMap,
                gBRDFIntegrationLUT);

    // HDR tonemapping
    color = color / (color + vec3(1.0));
    // gamma correct
    color = pow(color, vec3(1.0/2.2));

    FragColor = vec4(color, 1.);
    // FragColor = vec4(normalSample, 1.);

    // if(height < textureBlend[0])
    // {
    //     color = sampleFromTexture(0, texCoord);
    // }
    // else if(height >= textureBlend[textureCount - 1])
    // {
    //     color = sampleFromTexture(textureCount - 1, texCoord);
    // }
    // else
    // {
    //     for(int i=0; i < textureCount - 1; i++)
    //     {
    //         if(height >= textureBlend[i] && height < textureBlend[i + 1])
    //         {
    //             float b1 = height - textureBlend[i];
    //             float b2  = textureBlend[i + 1] - height;
    //             float blend = b1 / (b1 + b2);
    //             color = mix(sampleFromTexture(i, texCoord), sampleFromTexture(i + 1, texCoord), blend);
    //             break;
    //         }
    //     }
    // }

    // vec4 fragPosInLightSpace = lightSpaceMatrix * vec4(fragPos, 1.f);
	// float shadow = shadowCalculations(fragPosInLightSpace) * 0.8; // 0.8 to generate some ambient light

    // color = color * (1.0 - shadow);
}