#pragma once
#include <glm/glm.hpp>
#include "core/Core.h"
#include "component/Component.h"
#include "component/ComponentSerializer.h"
#include "serialize/CerealHelpers.h"

class EngineAPI PointLight : public Component
{
public:
	PointLight() = default;

	PointLight(glm::vec3 color, float intensity = 1.0f, float radius = 10.0f)
		: color(color), intensity(intensity), radius(radius)
	{
	}

	std::string getName() override { return "PointLight"; }

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(color);
		SERIALIZED_MEMBER_OPTIONAL2(intensity, 1.0f); // optional: scenes saved before it existed
		SERIALIZED_MEMBER_OPTIONAL2(radius, 10.0f);
	}

	glm::vec3 color = glm::vec3(1.0f);
	// Multiplies color before it reaches the shader, so color stays a plain [0,1] tint
	float intensity = 1.0f;
	// Light reaches zero at this distance
	float radius = 10.0f;
};

REGISTER_COMPONENT(PointLight)
