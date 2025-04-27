#frag

void frag(inout vec3 color)
{
    vec2 xy = uv * vec2(1, -1); // hack
    color = texture(MainTexture, xy).rgb;
}