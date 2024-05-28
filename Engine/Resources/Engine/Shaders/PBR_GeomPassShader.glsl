#vert

#version 330

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 norm;
layout (location = 2) in vec2 tex;

// ----- Definitions ----- //

#include ../../../../Engine/Resources/Engine/Shaders/include/defines.glsl
#include ../../../../Engine/Resources/Engine/Shaders/include/structs.glsl
#include ../../../../Engine/Resources/Engine/Shaders/include/uniforms.glsl
#include ../../../../Engine/Resources/Engine/Shaders/include/functions.glsl

// ----- Structs ----- //

// ----- Out ----- //

out VS_OUT {
    vec3 fragPos;
	vec3 normal;
    vec2 texCoord;
} vs_out;

// ----- Uniforms ----- //

// ----- Forward Declerations ----- //

// ----- Methods ----- //

#ifdef CUSTOM_SHADER
#custom_vert
#endif

void main()
{
	vec3 aPos = pos;
	vec3 aNorm = norm;
#ifdef CUSTOM_SHADER
	vert(aPos, aNorm);
#endif

	mat4 modelViewMat = view * model;
	vs_out.texCoord = tex;
	vs_out.normal = mat3(transpose(inverse(model))) * aNorm;
	vs_out.fragPos = (model * vec4(aPos, 1.0)).xyz;

	gl_Position = projection * modelViewMat * vec4(aPos, 1.0);
}

#frag

#version 330 

// ----- Definitions ----- //

#include ../../../../Engine/Resources/Engine/Shaders/include/defines.glsl
#include ../../../../Engine/Resources/Engine/Shaders/include/structs.glsl
#include ../../../../Engine/Resources/Engine/Shaders/include/uniforms.glsl
#include ../../../../Engine/Resources/Engine/Shaders/include/functions.glsl

// ----- In ----- //

in VS_OUT {
    vec3 fragPos;
	vec3 normal;
    vec2 texCoord;
} fs_in;

// ----- Out ----- //

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec3 gAlbedo;
layout (location = 3) out vec3 gMRA;

// ----- Uniforms ----- //
uniform PBR_Material material; 

// ----- Forward Declerations ----- //

// ----- Methods ----- //
 
void main() 
{ 	
	gPosition = fs_in.fragPos;
	gNormal = normalize(fs_in.normal) * texture(material.texture_normal, fs_in.texCoord).rgb;
	gAlbedo = texture(material.texture_albedo, fs_in.texCoord).rgb;
	gMRA.r = texture(material.texture_metallic, fs_in.texCoord).r;
	gMRA.g = texture(material.texture_roughness, fs_in.texCoord).r;
	gMRA.b = texture(material.texture_ao, fs_in.texCoord).r;
} 