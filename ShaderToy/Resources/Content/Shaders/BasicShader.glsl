#frag

// i need UV
// camera access
// 

uniform vec2 a;

void frag(inout vec3 color)
{
	color = vec3(uv.xy + a , 0.0);
}