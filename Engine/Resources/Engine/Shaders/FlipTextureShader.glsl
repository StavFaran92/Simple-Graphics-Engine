#vert

#version 330 core
layout (location = 0) in vec3 aPos;

out vec2 fragPos;

void main()
{
    vec3 pos = aPos;
    pos.y = -pos.y; // Flip the Y coordinate
    fragPos = (pos.xy + 1.0) / 2.0; // Transform from [-1, 1] to [0, 1] range
    gl_Position = vec4(aPos, 1.0); // Use original aPos for gl_Position
}

#frag

#version 330 core

in vec2 fragPos;

out vec4 color;

uniform sampler2D source;

void main()
{
    color = vec4(texture(source, fragPos).rgb, 1.0);
}