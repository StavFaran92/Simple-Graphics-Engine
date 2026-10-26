#vert

#version 330

layout (location = 0) in vec4 aVertex; // xy position in pixels, zw uv

uniform mat4 projection;
uniform vec2 offset;

out vec2 uv;

void main()
{
    uv = aVertex.zw;
    gl_Position = projection * vec4(aVertex.xy + offset, 0.0, 1.0);
}

#frag

#version 330

in vec2 uv;

uniform sampler2D atlas;
uniform vec4 color;
uniform float screenPxRange;

out vec4 outColor;

float median(float r, float g, float b)
{
    return max(min(r, g), min(max(r, g), b));
}

void main()
{
    vec3 s = texture(atlas, uv).rgb;
    float d = median(s.r, s.g, s.b) - 0.5;
    float alpha = clamp(d * screenPxRange + 0.5, 0.0, 1.0);
    outColor = vec4(color.rgb, color.a * alpha);
}
