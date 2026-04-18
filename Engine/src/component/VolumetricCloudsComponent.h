#pragma once

#include "core/Core.h"
#include "component/Component.h"
#include "component/ComponentSerializer.h"
#include "serialize/CerealHelpers.h"
#include "render/Material.h"

struct EngineAPI VolumetricCloudsComponent : public Component
{
	VolumetricCloudsComponent() = default;
	VolumetricCloudsComponent(Entity entity);

	std::string getName() override { return "VolumetricCloudsComponent"; }

	MaterialAssetRef getMaterial();

	template <class Archive>
	void serialize(Archive& archive) {
		//SERIALIZED_MEMBER(entity);
		//SERIALIZED_MEMBER(waterBodyNormal);
		//SERIALIZED_MEMBER(wave1Speed);
		//SERIALIZED_MEMBER(wave2Speed);
		//SERIALIZED_MEMBER(wave3Speed);
		//SERIALIZED_MEMBER(wave1Amp);
		//SERIALIZED_MEMBER(wave2Amp);
		//SERIALIZED_MEMBER(wave3Amp);
		//SERIALIZED_MEMBER(colorA);
		//SERIALIZED_MEMBER(colorB);
		//SERIALIZED_MEMBER(opacity);

	}

	Entity entity = Entity::EmptyEntity;

	glm::vec3 colorA{ 0.023497, 0.451692, 1.0 };
	glm::vec3 colorB{ 0.0, 0.05938, 0.135417 };
	float marchSize = 0.08;
};

REGISTER_COMPONENT(VolumetricCloudsComponent)