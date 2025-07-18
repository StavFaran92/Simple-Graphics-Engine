#include "lights/PointLight.h"

#include "render/Shader.h"
#include "core/Logger.h"
#include "component/Transformation.h"

PointLight::PointLight(glm::vec3 color, float aIntensity, float dIntensity, Attenuation attenuation)
	: Light(color, aIntensity, dIntensity), attenuation(attenuation)
{
	m_name = "pointLights";
}

void PointLight::SetAttenuation(Attenuation attenuation)
{
	attenuation = attenuation;
}

Attenuation PointLight::getAttenuation() const
{
	return attenuation;
}

void PointLight::useLight(Shader& shader, int index)
{
	Light::useLight(shader, index);

	
	shader.setUniformValue(m_name + "["+std::to_string(index) +"]"+ ".constant", attenuation.constant);
	shader.setUniformValue(m_name + "["+std::to_string(index) +"]"+ ".linear", attenuation.linear);
	shader.setUniformValue(m_name + "["+std::to_string(index) +"]"+ ".quadratic", attenuation.quadratic);
}