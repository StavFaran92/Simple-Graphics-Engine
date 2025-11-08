#frag

void frag(inout vec3 color)
{

    // color = vec3(1,1,0);

vec2 uv = gl_FragCoord.xy * texelSize;

    vec3 center = texture(MainTexture, uv).rgb;

    // Simple sobel-inspired edge detection on color
    float edgeStrength = 0.0;

    float threshold = 0.1; // Sensitivity

    vec3 neighbors[4];
    neighbors[0] = texture(MainTexture, uv + vec2(-texelSize.x, 0)).rgb;
    neighbors[1] = texture(MainTexture, uv + vec2(texelSize.x, 0)).rgb;
    neighbors[2] = texture(MainTexture, uv + vec2(0, -texelSize.y)).rgb;
    neighbors[3] = texture(MainTexture, uv + vec2(0, texelSize.y)).rgb;

    for (int i = 0; i < 4; ++i)
    {
        edgeStrength += distance(center, neighbors[i]);
    }

    edgeStrength = clamp(edgeStrength / 4.0, 0.0, 1.0);

    // If edge is strong enough, apply blur
    if (edgeStrength > threshold)
    {
        // 3x3 blur
        vec3 blurred = vec3(0.0);
        float weight = 1.0 / 9.0;
        for (int x = -1; x <= 1; ++x)
        {
            for (int y = -1; y <= 1; ++y)
            {
                vec2 offset = uv + vec2(x, y) * texelSize;
                blurred += texture(MainTexture, offset).rgb * weight;
            }
        }
        color = blurred;
    }
    else
    {
        color = center; // Leave unblurred
    }
    
    //color = texture(MainTexture, uv).rgb;
}