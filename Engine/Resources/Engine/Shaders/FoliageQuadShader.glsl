#vert

#version 430 

// ----- Definitions ----- //

#include ../../../../Engine/Resources/Engine/Shaders/include/defines.glsl
#include ../../../../Engine/Resources/Engine/Shaders/include/structs.glsl
#include ../../../../Engine/Resources/Engine/Shaders/include/uniforms.glsl
#include ../../../../Engine/Resources/Engine/Shaders/include/functions.glsl
                                                                                    
layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aTexcoords;

uniform vec3 patchPosition;
uniform vec2 patchSize;
uniform vec2 patchCount;



out vec3 fragPos;
out vec2 texCoords;
                                                                                    
void main()                                                                         
{ 
    fragPos = aPos;
    texCoords = aTexcoords;
    gl_Position = projection * view * model * vec4(aPos, 1.0); 
                                  
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
uniform sampler2D grassTexture;

in vec3 fragPos;
in vec2 texCoords;

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
    // vec3 bottomColor = vec3(0.1, 0.3, 0.1); // darker green
    // vec3 topColor    = vec3(0.4, 0.8, 0.3); // lighter green

    // // Set expected height range for blending
    // float minY = 0.0;
    // float maxY = 2.0; // adjust to match your scene/object scale

    // // Interpolation factor
    // float t = clamp((fragPos.y - minY) / (maxY - minY), 0.0, 1.0);

    // // Final graded color
    // vec3 color = mix(bottomColor, topColor, t);

    // vec3 L = normalize(-sunLightDir);
    // float diff = max(dot(normalize(Normal), L), 0.0);

    // color *= diff * 0.8 + 0.2; // keep a little base light

    vec4 texColor = texture(grassTexture, texCoords.xy);

    // Convert to grayscale brightness (or use texColor.g if it's a grass texture)
    float t = dot(texColor.rgb, vec3(0.299, 0.587, 0.114)); // standard luminance

    // Optional: Clamp or adjust range
    t = clamp(t, 0.0, 1.0);

    // Define your gradient
    vec3 bottomColor = vec3(0.1, 0.3, 0.1);
    vec3 topColor    = vec3(0.4, 0.8, 0.3);

    // Interpolate
    vec3 finalColor = mix(bottomColor, topColor, t);

    FragColor = vec4(finalColor, texColor.a);

    // FragColor = vec4(texture(grassTexture, texCoords.xy)); 
}