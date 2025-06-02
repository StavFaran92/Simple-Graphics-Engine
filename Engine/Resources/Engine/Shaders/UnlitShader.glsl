#vert

#version 330 

// ----- Definitions ----- //

#include ../../../../Engine/Resources/Engine/Shaders/include/defines.glsl
#include ../../../../Engine/Resources/Engine/Shaders/include/structs.glsl
#include ../../../../Engine/Resources/Engine/Shaders/include/uniforms.glsl
#include ../../../../Engine/Resources/Engine/Shaders/include/functions.glsl
                                                                                    
layout (location = 0) in vec3 aPos;                                              
                                                                                    
void main()                                                                         
{  
    gl_Position = projection * view * model * vec4(aPos, 1.0); 
                                  
}

#frag

#version 330 core

out vec4 FragColor;

uniform vec3 color;

void main() 
{
    FragColor = vec4(color, 1.0); 
}