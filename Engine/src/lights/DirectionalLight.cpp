#include "DirectionalLight.h"

#include "Shader.h"
#include "Logger.h"

DirectionalLight::DirectionalLight()

	: Light()
{
	m_name = "dirLight";
}

void DirectionalLight::useLight(Shader& shader, int index)
{
	Light::useLight(shader, index);

	//shader.setUniformValue(m_name + "[" + std::to_string(index) + "]" + ".direction", m_direction);
}