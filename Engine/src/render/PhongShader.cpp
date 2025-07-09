#include "PhongShader.h"

#include "PointLight.h"
#include "DirectionalLight.h"
#include "Transformation.h"

void PhongShader::updateDirLights(Shader* shader, entt::registry& registry)
{
	// Use all point lights
	auto view = registry.view<DirectionalLight>();

	int i = 0;
	for (auto it = view.begin(); it != view.end(); ++it, ++i)
	{
		auto& pLight = view.get<DirectionalLight>(*it);
		pLight.useLight(*shader, i);
	}
	shader->setUniformValue("dirLightCount", i);
}

void PhongShader::updatePointLights(Shader* shader, entt::registry& registry)
{
	// Use all point lights
	auto view = registry.view<PointLight, Transformation>();

	int i = 0;
	for (auto it = view.begin(); it != view.end(); ++it, ++i)
	{
		auto& pLight = view.get<PointLight>(*it);
		auto& transform = view.get<Transformation>(*it);
		shader->setUniformValue("pointLights[" + std::to_string(i) + "]" + ".position", transform.getLocalPosition());
		pLight.useLight(*shader, i);
	}
	shader->setUniformValue("pointLightCount", i);
}
