#frag

void frag(inout vec3 color)
{
    color = texture(MainTexture, uv).rgb;
}