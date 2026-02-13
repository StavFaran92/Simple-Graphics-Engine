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
    // gl_Position = vec4(pos.xy, 1.0, 1.0);  
    
    gl_Position = projection * view * model * vec4(aPos, 1.0); 
                                  
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

// ----- Out ----- //

out vec4 FragColor;

// ----- Uniforms ----- //
uniform vec3 cameraPos;
uniform vec3 cameraLookAt;
uniform float cameraFov;

uniform vec2 screenSize;

uniform vec3 u_aabbMin;
uniform vec3 u_aabbMax;

uniform sampler2D uMainTexture;

// ----- Methods ----- //

#ifdef CUSTOM_SHADER
#custom_frag
#endif

void main()
{
    vec4 color = vec4(0.0, 0.0, 0.0, 1.0);

#ifdef CUSTOM_SHADER
    frag(color);
#endif

    FragColor = color;
}
