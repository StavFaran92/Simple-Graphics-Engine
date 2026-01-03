#vert

#version 330 

// ----- Definitions ----- //

#include ../../../../Engine/Resources/Engine/Shaders/include/defines.glsl
#include ../../../../Engine/Resources/Engine/Shaders/include/structs.glsl
#include ../../../../Engine/Resources/Engine/Shaders/include/uniforms.glsl
#include ../../../../Engine/Resources/Engine/Shaders/include/functions.glsl
                                                                                    
layout (location = 0) in vec3 aPos;                                              

out vec2 uv;
                                                                                    
void main()                                                                         
{
    vec3 pos = aPos;
    uv = (pos.xy + 1.0) / 2.0; // Transform from [-1, 1] to [0, 1] range
    gl_Position = vec4(pos.xy, 1.0, 1.0);                                       
}

#frag

#version 330

// ----- Definitions ----- //

#include ../../../../Engine/Resources/Engine/Shaders/include/defines.glsl
#include ../../../../Engine/Resources/Engine/Shaders/include/structs.glsl
#include ../../../../Engine/Resources/Engine/Shaders/include/uniforms.glsl
#include ../../../../Engine/Resources/Engine/Shaders/include/functions.glsl

// ----- In ----- //

// UV [0,1]
in vec2 uv;

uniform sampler2D uMainTexture;
uniform sampler2D uVolumeColor;

// ----- Out ----- //

out vec4 FragColor;

// ----- Methods ----- //

void main()
{
    vec4 volumeColor = texture(uVolumeColor, uv).rgba;
    vec3 sceneColor = texture(uMainTexture, uv).rgb;

    vec3 color = mix(sceneColor, volumeColor.xyz, clamp(volumeColor.w, 0.0, 1.0));

    FragColor = vec4(color, 1.0);
}
