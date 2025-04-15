// ----- Uniforms buffers ----- //

layout (std140) uniform Time
{
    float iTime;
};

layout (std140) uniform Lights
{
	int pointLightCount;
	int dirLightCount;
	PointLight pointLights[NR_POINT_LIGHTS];
	DirLight dirLight[NR_DIR_LIGHT]; 
};

// ----- Uniforms ----- //

uniform sampler2D shadowMap;
uniform mat4 model;
uniform mat4 projection;
uniform mat4 view;
uniform mat4 lightSpaceMatrix;
uniform bool isGpuInstanced;
uniform bool isAnimated;