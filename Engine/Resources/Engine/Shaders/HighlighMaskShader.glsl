#vert

#version 330 
                                                                                    
layout (location = 0) in vec3 aPos;                                              
                                                                                    
void main()                                                                         
{  
    gl_Position = projection * view * model * vec4(aPos, 1.0); 
                                  
}

#frag

#version 330 core

out vec4 FragColor;

void main() 
{
    FragColor = vec4(1.0); // white on black background
}