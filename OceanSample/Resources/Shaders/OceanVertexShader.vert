#version 330

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 norm;
layout (location = 2) in vec2 tex;
layout (location = 3) in vec3 color;

out vec3 Color;
out vec2 texCoord;
out vec3 Normal;
out vec3 FragPos;

uniform mat4 model;
uniform mat4 projection;
uniform mat4 view;
uniform float time;

uniform bool isGpuInstanced = false;

void main()
{
	mat4 modelMatrix =  model;

	vec3 vPos = pos;
	vPos.z = sin(pos.x * 2  + time);

	gl_Position = projection * view * modelMatrix * vec4(vPos, 1.0);
	Color = color;
	
	texCoord = tex;

	Normal = mat3(transpose(inverse(modelMatrix))) * norm;

	FragPos = (modelMatrix * vec4(vPos, 1.0)).xyz;
}