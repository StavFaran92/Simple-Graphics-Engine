#frag

void frag(inout vec3 color)
{
    vec2 screenPos = gl_FragCoord.xy * vec2(1, -1); // hack
    vec2 screenUV = screenPos / screenSize;

    color = texture(MainTexture, screenUV.xy).rgb;

    if(color.r == 0) color = vec3(1,0,0);
}