#vert

void vert(inout vec3 aPos, inout vec3 aNorm)
{
}

#frag

uniform vec3 a;
uniform sampler2D myTexture;

void frag(inout vec3 color)
{
	color = texture(myTexture, getTexCoords()).rgb;
}