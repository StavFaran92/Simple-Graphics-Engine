#vert

void vert(inout vec3 aPos, inout vec3 aNorm)
{
}

#frag

uniform vec3 a;

void frag(inout vec3 color)
{
	color = a;
}