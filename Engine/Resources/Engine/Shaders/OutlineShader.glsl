#vert

#version 330

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 norm;
layout (location = 2) in vec2 tex;
layout (location = 3) in vec3 color;
layout (location = 6) in mat4 instanceMatrix;

out vec3 Color;
out vec2 texCoord;
out vec3 Normal;
out vec3 FragPos;

uniform mat4 model;
uniform mat4 projection;
uniform mat4 view;

void main()
{
	mat4 modelMatrix =  model;
	gl_Position = projection * view * modelMatrix * vec4(pos, 1.0);
	Color = color;
	
	texCoord = tex;

	Normal = mat3(transpose(inverse(modelMatrix))) * norm;

	FragPos = (modelMatrix * vec4(pos, 1.0)).xyz;
}

#frag

#version 330 core
out vec4 FragColor;

void main()
{
    FragColor = vec4(0.04, 0.28, 0.26, 1.0);
}