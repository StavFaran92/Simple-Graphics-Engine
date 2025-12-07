#vert

#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 4) in vec3 aTangent;

out VS_OUT {
    vec3 normal;
    vec3 tangent;
} vs_out;

uniform mat4 view;
uniform mat4 model;

void main()
{
    gl_Position = view * model * vec4(aPos, 1.0); 
    mat3 normalMatrix = mat3(transpose(inverse(view * model)));
    vs_out.normal = normalize(vec3(vec4(normalMatrix * aNormal, 0.0)));
    vs_out.tangent = normalize(vec3(vec4(normalMatrix * aTangent, 0.0)));

}

#geom

#version 330 core
layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

in VS_OUT {
    vec3 normal;
    vec3 tangent;
} gs_in[];

const float MAGNITUDE = 0.4;
  
uniform mat4 projection;

void GenerateNormalLine(int index)
{
    gl_Position = projection * gl_in[index].gl_Position;
    EmitVertex();
    gl_Position = projection * (gl_in[index].gl_Position + 
                                vec4(gs_in[index].normal, 0.0) * MAGNITUDE);
    EmitVertex();
    EndPrimitive();
}


void GenerateTangentLine(int index)
{
    gl_Position = projection * gl_in[index].gl_Position;
    EmitVertex();
    gl_Position = projection * (gl_in[index].gl_Position + 
                                vec4(gs_in[index].tangent, 0.0) * MAGNITUDE);
    EmitVertex();
    EndPrimitive();
}


void main()
{
    // GenerateNormalLine(0); // first vertex normal
    // GenerateNormalLine(1); // second vertex normal
    // GenerateNormalLine(2); // third vertex normal

    GenerateTangentLine(0); // first vertex normal
    GenerateTangentLine(1); // second vertex normal
    GenerateTangentLine(2); // third vertex normal
    // GenerateTangentLine(3); // third vertex normal
}  

#frag

#version 330 core
out vec4 FragColor;

void main()
{
	FragColor = vec4(1.0, 1.0, 0.0, 1.0);
}
