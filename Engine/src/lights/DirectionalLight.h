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

	std::string getName() override { return "DirectionalLight"; }

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(color);
		SERIALIZED_MEMBER_OPTIONAL2(intensity, 1.0f); // optional: scenes saved before it existed
	}

	// Multiplies color before it reaches the shader, so color stays a plain [0,1] tint
	float intensity = 1.0f;
};

REGISTER_COMPONENT(DirectionalLight)
