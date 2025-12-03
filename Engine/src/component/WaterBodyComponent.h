#pragma once

#include "core/Core.h"
#include "component/Component.h"
#include "component/ComponentSerializer.h"
#include "serialize/CerealHelpers.h"

struct EngineAPI WaterBodyComponent : public Component
{
	WaterBodyComponent() = default;
	WaterBodyComponent(Entity entity);

	static void attachToEntity(std::shared_ptr<Component> c, Entity entityHandler, Scene& scene)
	{
		attachSimple<WaterBodyComponent>(c, entityHandler);
	}

	std::shared_ptr<TextureSampler> getWaterNormalSampler();

	Entity entity = Entity::EmptyEntity;
};

REGISTER_COMPONENT(WaterBodyComponent)