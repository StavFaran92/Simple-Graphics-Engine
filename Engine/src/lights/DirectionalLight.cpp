#include "lights/DirectionalLight.h"

#include "render/Shader.h"
#include "core/Logger.h"
#include "runtime/Scene.h"

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