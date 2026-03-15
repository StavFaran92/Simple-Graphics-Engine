#pragma once

#include "core/Core.h"
#include "component/Component.h"
#include "component/ComponentSerializer.h"
#include "serialize/CerealHelpers.h"
#include "render/Material.h"

struct EngineAPI WaterBodyComponent : public Component
{
	WaterBodyComponent() = default;
	WaterBodyComponent(Entity entity);

	static void attachToEntity(std::shared_ptr<Component> c, Entity entityHandler, ResourceWrapper<Scene>& scene);

	AssetHandle<MaterialAsset> getMaterial();

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(entity);
		SERIALIZED_MEMBER(waterBodyNormal);
		SERIALIZED_MEMBER(wave1Speed);
		SERIALIZED_MEMBER(wave2Speed);
		SERIALIZED_MEMBER(wave3Speed);
		SERIALIZED_MEMBER(wave1Amp);
		SERIALIZED_MEMBER(wave2Amp);
		SERIALIZED_MEMBER(wave3Amp);
		SERIALIZED_MEMBER(colorA);
		SERIALIZED_MEMBER(colorB);
		SERIALIZED_MEMBER(opacity);

	}

	Entity entity = Entity::EmptyEntity;

	std::shared_ptr<TextureSampler> waterBodyNormal;

	glm::vec2 wave1Speed{ 0.1, 0.01 };
	glm::vec2 wave2Speed{ 0.01, -0.1 };
	glm::vec2 wave3Speed{ 0.01, -0.01 };

	float wave1Amp = 3.0;
	float wave2Amp = 3.5;
	float wave3Amp = 1.0;

	glm::vec3 colorA{ 0.023497, 0.451692, 1.0 };
	glm::vec3 colorB{ 0.0, 0.05938, 0.135417 };

	float opacity = 1.0;
};

REGISTER_COMPONENT(WaterBodyComponent)