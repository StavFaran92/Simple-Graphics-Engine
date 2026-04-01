#pragma once
#include "lights/Light.h"
#include "core/Core.h"
#include "serialize/CerealHelpers.h"


class EngineAPI DirectionalLight : public Light
{
public:
	DirectionalLight();

	DirectionalLight(glm::vec3 color, glm::vec3 dir, float aIntensity, float dIntensity)
		: Light(color, aIntensity, dIntensity)
	{
		m_name = "dirLight";
	}

	void useLight(Shader& shader, int index) override;

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(color);
	}
};

REGISTER_COMPONENT(DirectionalLight)
