#vert

#version 330 core
layout (location = 0) in vec3 aPos;

out vec2 fragPos;

void main()
{
    vec3 pos = aPos;
    fragPos = (pos.xy + 1.0) / 2.0; // Transform from [-1, 1] to [0, 1] range
    gl_Position = vec4(aPos, 1.0); // Use original aPos for gl_Position
}

#frag

#version 330 core

in vec2 fragPos;

out vec4 color;

uniform sampler2D sampler0;
uniform sampler2D sampler1;
uniform sampler2D sampler2;
uniform sampler2D sampler3;

uniform int channel0;
uniform int channel1;
uniform int channel2;
uniform int channel3;

float sampleChannel(sampler2D tex, vec2 uv, int channel)
{
    vec4 s = texture(tex, uv);
    if (channel == 0) return s.r;
    if (channel == 1) return s.g;
    if (channel == 2) return s.b;
    return s.a;
}

void main()
{
    color = vec4(
        sampleChannel(sampler0, fragPos, channel0),
        sampleChannel(sampler1, fragPos, channel1),
        sampleChannel(sampler2, fragPos, channel2),
        sampleChannel(sampler3, fragPos, channel3)
    );
}