#frag

uniform sampler2D uMaskTex;
uniform vec2 uTexelSize; // = 1.0 / screen resolution

void frag(inout vec3 color) 
{
    vec2 uv = gl_FragCoord.xy * uTexelSize;

    float center = texture(uMaskTex, uv).r;

    // Sample 4 neighbors
    float left  = texture(uMaskTex, uv + vec2(-uTexelSize.x, 0)).r;
    float right = texture(uMaskTex, uv + vec2( uTexelSize.x, 0)).r;
    float up    = texture(uMaskTex, uv + vec2(0, -uTexelSize.y)).r;
    float down  = texture(uMaskTex, uv + vec2(0,  uTexelSize.y)).r;

    // If any neighbor is different → edge
    float edge = step(0.1, abs(center - left) + abs(center - right) + abs(center - up) + abs(center - down));

    color = vec3(edge);
}