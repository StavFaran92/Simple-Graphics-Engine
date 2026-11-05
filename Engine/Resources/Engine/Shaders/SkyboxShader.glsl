#vert

#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords3D;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    TexCoords3D = aPos;
    mat4 rotView = mat4(mat3(view)); // remove translation from the view matrix
    vec4 clipPos = projection * rotView * vec4(aPos, 1.0);
    gl_Position = clipPos.xyww;
}  

#frag

#version 330 core

uniform samplerCube skybox;
in vec3 TexCoords3D;
out vec4 color;

const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y)); // theta and phi
    uv *= invAtan; // divide by 2PI and PI to transform to [-.5, .5]
    uv += 0.5; // convert to [0, 1]
    return uv;
}

void main()
{    
    // The cubemap is linear, the same one the lighting uses
    color = vec4(pow(texture(skybox, TexCoords3D).rgb, vec3(1.0 / 2.2)), 1.0);
}