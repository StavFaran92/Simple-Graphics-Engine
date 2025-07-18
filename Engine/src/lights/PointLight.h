#pragma once
#include "Light.h"
#include "Core.h"


struct EngineAPI Attenuation {
	float constant = 1;
	float linear = .35f;
	float quadratic = .44f;

	template <class Archive>
	void serialize(Archive& archive) {
		archive(constant, linear, quadratic);
	}
};

class EngineAPI PointLight : public Light
{
public:
	PointLight() :
		Light(), m_attenuation()
	{
		m_name = "pointLights";
	}

	PointLight(glm::vec3 color, float aIntensity, float dIntensity, Attenuation attenuation);

	void useLight(Shader& shader, int index);

	void SetAttenuation(Attenuation attenuation);
	Attenuation getAttenuation() const;

	template <class Archive>
	void serialize(Archive& archive) {
		archive(m_attenuation, m_color);
	}

private:
	Attenuation m_attenuation;
};
