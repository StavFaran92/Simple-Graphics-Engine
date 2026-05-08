#compute

#version 430

// Binding 0: Heightmap texture (R32F)
layout(r32f, binding = 0) uniform image2D target;

uniform vec2 brushCenter;     // pixel coords
uniform float brushRadius;
uniform float brushStrength;

layout (local_size_x = 16, local_size_y = 16) in;

void main()
{
    ivec2 pix = ivec2(gl_GlobalInvocationID.xy);

    float dist = distance(vec2(pix), brushCenter);
    if (dist > brushRadius)
        return;

    float falloff = 1.0 - (dist / brushRadius);

    float h = imageLoad(target, pix).r;
    h += brushStrength * falloff;

    h = clamp(h, 0.0, 1.0);

    imageStore(target, pix, vec4(h, 0, 0, 0));
}
