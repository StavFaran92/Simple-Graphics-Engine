#pragma once
#include "lights/Light.h"
#include "core/Core.h"
#include "serialize/CerealHelpers.h"

struct EngineAPI Attenuation {
	float constant = 1;
	float linear = .35f;
	float quadratic = .44f;

	template <class Archive>
	void serialize(Archive& archive) {
			SERIALIZED_MEMBER(constant); 
			SERIALIZED_MEMBER(linear);
			SERIALIZED_MEMBER(quadratic);
	}
};

class EngineAPI PointLight : public Light
{
public:
	PointLight() :
		Light(), attenuation()
	{
		m_name = "pointLights";
	}

	PointLight(glm::vec3 color, float aIntensity, float dIntensity, Attenuation attenuation);

	void useLight(Shader& shader, int index);

	void SetAttenuation(Attenuation attenuation);
	Attenuation getAttenuation() const;

	static void attachToEntity(std::shared_ptr<Component>, Entity, ResourceWrapper<Scene>&);

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(attenuation);
		SERIALIZED_MEMBER(color);
	}

public:
	Attenuation attenuation;
	
};

REGISTER_COMPONENT(PointLight)
