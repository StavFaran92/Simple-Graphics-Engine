#frag

// i need UV
// camera access
// 

uniform vec2 a;
uniform vec2 b;

void frag(inout vec3 color)
{
	color = vec3(uv.xy + a + b , 0.0);
}