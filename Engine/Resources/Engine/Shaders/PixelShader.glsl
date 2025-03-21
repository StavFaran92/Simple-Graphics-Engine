#vert

#version 330

layout (location = 0) in vec3 pos;

// ----- Out ----- //

out vec3 fragPos;

// ----- Uniforms ----- //

void main()
{
    fragPos = pos;

    gl_Position = vec4(pos, 1.0);
}

#frag

#version 330

// ----- In ----- //

out vec3 fragPos;

// ----- Out ----- //

out vec4 FragColor;

// ----- Uniforms ----- //

// ----- Methods ----- //

#ifdef CUSTOM_SHADER
#custom_frag
#endif

void main()
{
    vec3 color = vec3(0.0, 0.0, 0.0);

#ifdef CUSTOM_SHADER
    frag(color);
#endif

    FragColor = vec4(color, 1.0);
}
