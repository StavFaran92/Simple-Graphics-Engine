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

void DirectionalLight::attachToEntity(std::shared_ptr<Component> c, Entity entityHandler, Scene& scene)
{
	(void)scene;
	if (auto dl = std::dynamic_pointer_cast<DirectionalLight>(c))
	{
		entityHandler.addComponent<DirectionalLight>(*dl);
	}
}
