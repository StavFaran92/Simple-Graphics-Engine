#frag

void frag(inout vec3 color)
{
    vec2 xy = gl_FragCoord.xy * vec2(1, -1); // hack

    vec2 screenPos = xy; // (x,y) in pixel units

    // If you want it normalized to 0..1:
    vec2 screenUV = screenPos / vec2(1920, 1080);

    color = texture(MainTexture, screenUV.xy).rgb;

    if(color.r == 0) color = vec3(1,0,0);
}