#frag

#version 330 core
out vec4 FragColor;

uniform sampler2D uEdgeTex;
uniform vec2 uTexelSize;
uniform vec3 uHighlightColor;
uniform int uDilationRadius;

void frag(inout vec3 color)
{
    vec2 uv = gl_FragCoord.xy * uTexelSize;

    float edge = 0.0;
    for (int y = -uDilationRadius; y <= uDilationRadius; ++y) {
        for (int x = -uDilationRadius; x <= uDilationRadius; ++x) {
            vec2 offset = vec2(x, y) * uTexelSize;
            edge += texture(uEdgeTex, uv + offset).r;
        }
    }

    edge = clamp(edge, 0.0, 1.0);
    vec3 sceneColor = texture(MainTexture, uv).rgb;
    vec3 finalColor = mix(sceneColor, uHighlightColor, edge);

    color = finalColor;
}