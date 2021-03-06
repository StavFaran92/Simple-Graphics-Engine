#version 330
//--------- In ---------
in vec3 Color;
in vec2 texCoord;
in vec3 Normal;
in vec3 FragPos;

//--------- Out ---------
out vec4 colour;

//--------- Data structures ---------
struct Material {
	// textures
	sampler2D texture_diffuse1;
	sampler2D texture_diffuse2;
	sampler2D texture_diffuse3;
	sampler2D texture_specular1;
	sampler2D texture_specular2;
    float shininess;
}; 

struct DirLight {
    vec3 direction;
    vec3 color;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};  

struct PointLight {    
    vec3 position;
    vec3 color;
    
    float constant;
    float linear;
    float quadratic;  

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};  
#define NR_POINT_LIGHTS 16  
#define NR_DIR_LIGHT 16  
  
//--------- Uniforms ---------
// material
uniform Material material;

// light
uniform DirLight dirLight[NR_DIR_LIGHT];
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform int pointLightCount;
uniform int dirLightCount;

// camera
uniform vec3 viewPos;

// use Colors
uniform bool useColors = false;

uniform vec4 colorMul;

// Forward declerations
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec2 aTexCoord);  
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec2 aTexCoord);

//--------- Source ---------
void main()
{
    // properties
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 result;
    // Directional lighting
    for(int i = 0; i < dirLightCount; i++)
        result += CalcDirLight(dirLight[i], norm, viewDir, texCoord);

    // Point lights
    for(int i = 0; i < pointLightCount; i++)
        result += CalcPointLight(pointLights[i], norm, FragPos, viewDir, texCoord);        
    
    if(useColors)
        result *= Color;

    colour = vec4(result, 1.0) + colorMul;
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec2 aTexCoord)
{
    vec3 lightDir = normalize(-light.direction);

    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);

    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    // combine results
    vec3 ambient  = light.ambient  * vec3(texture(material.texture_diffuse1, aTexCoord));
    vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.texture_diffuse1, aTexCoord));
    vec3 specular = light.specular * spec * vec3(texture(material.texture_specular1, aTexCoord));
    return (ambient + diffuse + specular) * light.color;
} 

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec2 aTexCoord)
{
    vec3 lightDir = normalize(light.position - fragPos);

    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);

    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    // attenuation
    float distance    = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
  			     light.quadratic * (distance * distance));    

    // combine results
    vec3 ambient  = light.ambient  * vec3(texture(material.texture_diffuse1, aTexCoord));
    vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.texture_diffuse1, aTexCoord));
    vec3 specular = light.specular * spec * vec3(texture(material.texture_specular1, aTexCoord));
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular) * light.color;
} 