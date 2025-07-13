#vert

#version 330 

// ----- Definitions ----- //

#include ../../../../Engine/Resources/Engine/Shaders/include/defines.glsl
#include ../../../../Engine/Resources/Engine/Shaders/include/structs.glsl
#include ../../../../Engine/Resources/Engine/Shaders/include/uniforms.glsl
#include ../../../../Engine/Resources/Engine/Shaders/include/functions.glsl
                                                                                    
layout (location = 0) in vec3 aPos;
layout (location = 4) in vec3 instancePos;                                    
                                                                                    
void main()                                                                         
{ 
    mat4 model = mat4(1.0);
    //model[3] = vec4(instancePos, 1.0);
    gl_Position = projection * view * model * vec4(aPos, 1.0); 
                                  
}

#frag

#version 330 core

out vec4 FragColor;

// uniform vec3 color;

void main() 
{
    FragColor = vec4(0.0, 1.0, 0.0, 1.0); 
}