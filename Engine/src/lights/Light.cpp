#include "Light.h"
#include <GL/glew.h>
#include "ApplicationConstants.h"
#include "Transformation.h"
#include "Shader.h"
#include "Logger.h"

Light::Light()
	:m_color(glm::vec3(1.0f, 1.0f, 1.0f)), m_ambientIntensity(0.2f), m_diffuseIntensity(.5f)
{}

Light::Light(glm::vec3 color, float aIntensity, float dIntensity) :
	m_color(color), m_ambientIntensity(aIntensity), m_diffuseIntensity(dIntensity)
{}

void Light::useLight(Shader& shader, int index)
{
	shader.setUniformValue(m_name + "[" +std::to_string(index) + "]" + ".color", m_color);
	shader.setUniformValue(m_name + "[" +std::to_string(index) + "]" + ".ambient", Constants::VEC3_ONE * m_ambientIntensity);
	shader.setUniformValue(m_name + "[" +std::to_string(index) + "]" + ".diffuse", Constants::VEC3_ONE * m_diffuseIntensity); // darken diffuse light a bit
	shader.setUniformValue(m_name + "[" +std::to_string(index) + "]" + ".specular", Constants::VEC3_ONE);
}

void Light::SetAmbientIntensity(float intensity)
{
	m_ambientIntensity = intensity;
}

void Light::SetDiffuseIntensity(float intensity)
{
	m_diffuseIntensity = intensity;
}

void Light::SetColor(glm::vec3 color)
{
	m_color = color;
}

glm::vec3 Light::getColor() const
{
	return m_color;
}

Light::~Light()
{
}