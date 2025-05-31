// #vert

// #version 330 core
// layout (location = 0) in vec3 aPos;


// void main()
// {
//     gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0); 
// }  

#frag

#version 330 core
out vec4 FragColor;

uniform sampler2D uMaskTex;
uniform vec2 uTexelSize; // = 1.0 / screen resolution

void main() 
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

    FragColor = vec4(vec3(edge), 1.0);
}