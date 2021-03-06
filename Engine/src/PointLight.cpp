#include "PointLight.h"

#include "Shader.h"
#include "Logger.h"
#include "Transformation.h"

PointLight::PointLight(glm::vec3 color, glm::vec3 pos, float aIntensity, float dIntensity, Attenuation attenuation)
	: Light(color, aIntensity, dIntensity), m_attenuation(attenuation)
{
	m_name = "pointLights";

	m_transform->setPosition(pos);
}

void PointLight::SetAttenuation(Attenuation attenuation)
{
	m_attenuation = attenuation;
}

void PointLight::useLight(Shader& shader, int index)
{
	Light::useLight(shader, index);

	shader.setFloat(m_name + "["+std::to_string(index) +"]"+ ".position", m_transform->getPosition());
	shader.setFloat(m_name + "["+std::to_string(index) +"]"+ ".constant", m_attenuation.constant);
	shader.setFloat(m_name + "["+std::to_string(index) +"]"+ ".linear", m_attenuation.linear);
	shader.setFloat(m_name + "["+std::to_string(index) +"]"+ ".quadratic", m_attenuation.quadratic);
}