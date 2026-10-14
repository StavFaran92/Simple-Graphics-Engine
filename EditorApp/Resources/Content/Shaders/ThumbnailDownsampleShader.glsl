#vert

#version 330 core
layout (location = 0) in vec3 aPos;

out vec2 uv;

void main()
{
    uv = (aPos.xy + 1.0) / 2.0; // Transform from [-1, 1] to [0, 1] range
    gl_Position = vec4(aPos.xy, 0.0, 1.0);
}

#frag

#version 330 core

in vec2 uv;

out vec4 color;

uniform sampler2D source;
uniform vec2 footprint;   // Source UV extent covered by one output pixel
uniform int taps;         // Samples per axis per output pixel
uniform bool flipY;       // Source was stored bottom-up
uniform vec3 background;  // Shown through transparent texels

// Box filter: average a taps x taps grid spread over the output pixel's footprint
void main()
{
    vec2 center = vec2(uv.x, flipY ? 1.0 - uv.y : uv.y);

    vec4 sum = vec4(0.0);
    for (int y = 0; y < taps; ++y)
    {
        for (int x = 0; x < taps; ++x)
        {
            vec2 offset = (vec2(x, y) + 0.5) / float(taps) - 0.5;
            sum += texture(source, center + offset * footprint);
        }
    }
    sum /= float(taps * taps);

    color = vec4(mix(background, sum.rgb, sum.a), 1.0);
}
