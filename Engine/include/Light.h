#pragma once
#include <glm/glm.hpp>

#include <string>
#include "Core.h"
#include "Component.h"

class Shader;

class EngineAPI Light : public Component
{
public:
	Light();
	Light(glm::vec3 color, float aIntensity, float dIntensity);

	virtual void useLight(Shader& shader, int index);

	void SetAmbientIntensity(float intensity);
	void SetDiffuseIntensity(float intensity);
	void SetColor(glm::vec3 color);
	glm::vec3 getColor() const;

	virtual ~Light();

protected:
	glm::vec3 m_color;
	float m_ambientIntensity = 0;
	float m_diffuseIntensity = 0;
	std::string m_name;
};
