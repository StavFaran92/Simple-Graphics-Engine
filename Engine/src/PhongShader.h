#pragma once
#include <string>
#include <memory>

#include "Shader.h"

class DirectionalLight;
class PointLight;

class PhongShader : public Shader
{
public:
	PhongShader();

	void setDirLightCount(int count);
	void setPointLightCount(int count);
	void setUseColors(bool enable);
	void setViewPos(glm::vec3 viewPosition);
	void setColorMul(glm::vec4 colorMul);
	void updateDirLights(std::unordered_map<uint32_t, std::shared_ptr<DirectionalLight>>& dirLights);
	void updatePointLights(std::unordered_map<uint32_t, std::shared_ptr<PointLight>>& pointLights);
};

