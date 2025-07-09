#include "PointLight.h"

#include "Shader.h"
#include "Logger.h"
#include "Transformation.h"

PointLight::PointLight(glm::vec3 color, float aIntensity, float dIntensity, Attenuation attenuation)
	: Light(color, aIntensity, dIntensity), m_attenuation(attenuation)
{
	m_name = "pointLights";
}

void PointLight::SetAttenuation(Attenuation attenuation)
{
	m_attenuation = attenuation;
}

Attenuation PointLight::getAttenuation() const
{
	return m_attenuation;
}

void PointLight::useLight(Shader& shader, int index)
{
	Light::useLight(shader, index);

	
	shader.setUniformValue(m_name + "["+std::to_string(index) +"]"+ ".constant", m_attenuation.constant);
	shader.setUniformValue(m_name + "["+std::to_string(index) +"]"+ ".linear", m_attenuation.linear);
	shader.setUniformValue(m_name + "["+std::to_string(index) +"]"+ ".quadratic", m_attenuation.quadratic);
}