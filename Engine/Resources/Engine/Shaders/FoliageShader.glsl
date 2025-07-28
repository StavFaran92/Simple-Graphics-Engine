#vert

#version 430 

// ----- Definitions ----- //

#include ../../../../Engine/Resources/Engine/Shaders/include/defines.glsl
#include ../../../../Engine/Resources/Engine/Shaders/include/structs.glsl
#include ../../../../Engine/Resources/Engine/Shaders/include/uniforms.glsl
#include ../../../../Engine/Resources/Engine/Shaders/include/functions.glsl
                                                                                    
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 norm;
// layout (location = 4) in vec3 instancePos; 

// layout(std430, binding = 0) buffer InstanceData {
//     vec4 instancePos[];
// };

layout(std140, binding = 4) uniform RandomPatchSample { // todo give better name
    vec4 randomPatchSample[255]; 
};

// layout(std140, binding = 1) uniform PatchOffset { // todo give better name
//     vec4 patchOffset[10*10]; 
// };

uniform vec3 patchPosition;
uniform vec2 patchSize;
uniform vec2 patchCount;
uniform vec2 patchID;
uniform sampler2D windNoise;

uniform float time;

out vec3 Normal;
out vec3 fragPos;
                                                                                    
void main()                                                                         
{ 
    int insID = gl_InstanceID;
    int invocationID = insID % 255;
    vec4 patchOffset = vec4(patchID.x, 0.0, patchID.y, 0.0);
    vec4 vPos = randomPatchSample[invocationID] * vec4(patchSize.x, 0, patchSize.y, 0) + vec4(patchPosition, 0.0) + patchOffset;

    float posX = vPos[0];
    float posZ = vPos[2];
    float height = vPos[1];

    vec2 windUV = vec2(posX, posZ) * 0.05 + vec2(time * 0.02, time * 0.02);
    vec4 wind = texture(windNoise, windUV);

    vPos[0] += wind.r * aPos.y;
    vPos[2] += wind.g * aPos.y;

    mat4 localModel = mat4(1.0);
    localModel[3] = vPos;
    localModel[3][3] = 1;
    Normal = norm;
    fragPos = aPos;
    gl_Position = projection * view * localModel * vec4(aPos, 1.0); 
                                  
}

#frag

#version 330 core

#include ../../../../Engine/Resources/Engine/Shaders/include/defines.glsl
#include ../../../../Engine/Resources/Engine/Shaders/include/structs.glsl
#include ../../../../Engine/Resources/Engine/Shaders/include/uniforms.glsl
#include ../../../../Engine/Resources/Engine/Shaders/include/functions.glsl

out vec4 FragColor;

uniform vec3 colorA;
uniform vec3 colorB;
uniform vec3 viewDir;

in vec3 Normal;
in vec3 fragPos;

// uniform vec3 color;

void main() 
{
    // Constants (grass-like material)
    const vec3 ambientCoeff  = vec3(0.1, 0.2, 0.1);  // soft green ambient
    const vec3 diffuseCoeff  = vec3(0.3, 0.6, 0.2);  // strong greenish diffuse
    const vec3 specularCoeff = vec3(0.05);           // very low specular
    const float shininess    = 32.0;                  // low shininess

    // const vec3 sunLightDir = dirLight[0].direction.xyz;
    // const vec3 sunLightColor = dirLight[0].color.rgb;
    const vec3 sunLightDir = vec3(0, -1, 0);
    // const vec3 sunLightColor = vec3(1, 1, 1);

    // // Normalize vectors
    // vec3 N = normalize(Normal);
    // vec3 V = normalize(viewDir);
    // vec3 L = normalize(-sunLightDir); // Direction *to* light

    // // Ambient
    // vec3 ambient = ambientCoeff * sunLightColor;

    // // Diffuse
    // float diff = max(dot(N, L), 0.0);
    // vec3 diffuse = diffuseCoeff * sunLightColor * diff;

    // // Specular
    // vec3 R = reflect(-L, N);
    // float spec = pow(max(dot(V, R), 0.0), shininess);
    // vec3 specular = specularCoeff * sunLightColor * spec;

    // // Final color
    // vec3 color = ambient + diffuse + specular;

    // Base grass color
    vec3 bottomColor = colorA; // darker green
    vec3 topColor    = colorB; // lighter green

    // Set expected height range for blending
    float minY = 0.0;
    float maxY = 1.5; // adjust to match your scene/object scale

    // Interpolation factor
    float t = clamp((fragPos.y - minY) / (maxY - minY), 0.0, 1.0);

    // Final graded color
    vec3 color = mix(bottomColor, topColor, t);

    // vec3 L = normalize(-sunLightDir);
    // float diff = max(dot(normalize(Normal), L), 0.0);

    // color *= diff * 0.8 + 0.2; // keep a little base light

    FragColor = vec4(color, 1.0); 
}