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
    model[3] = vec4(instancePos, 1.0);
    gl_Position = projection * view * model * vec4(aPos, 1.0); 
                                  
}

#frag

#version 330 core

out vec4 FragColor;

uniform vec3 colorA;
uniform vec3 colorB;

// uniform vec3 color;

void main() 
{

    // for(int i = 0; i < dirLightCount; ++i)
	// {
    //     vec3 ambient = 0.0f;
	// 	vec3 diffuse = dirLight[i].direction
	// }

    vec3 color = mix(colorA, colorB, gl_FragCoord.y);

    FragColor = vec4(color, 1.0); 
}