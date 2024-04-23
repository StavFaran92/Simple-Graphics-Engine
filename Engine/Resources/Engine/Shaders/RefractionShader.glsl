#vert

#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aTextureCoords;

out vec3 Normal;
out vec3 Position;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    Normal = mat3(transpose(inverse(model))) * aNormal;
    Position = vec3(model * vec4(aPos, 1.0));
    gl_Position = projection * view * vec4(Position, 1.0);
}  

#frag

#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 Position;

uniform vec3 viewPos;
uniform samplerCube skybox;
uniform float refractiveRatio;

void main()
{             
    vec3 I = normalize(Position - viewPos);
    vec3 R = refract(I, normalize(Normal), refractiveRatio);
    FragColor = vec4(texture(skybox, R).rgb, 1.0);
} 