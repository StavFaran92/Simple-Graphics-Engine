#pragma once
#include <string>
#include <memory>

#include "Shader.h"
#include <entt/entt.hpp>

class DirectionalLight;
class PointLight;

class PhongShader
{
public:
	static void updateDirLights(Shader* shader, entt::registry& registry);
	static void updatePointLights(Shader* shader, entt::registry& registry);
};

