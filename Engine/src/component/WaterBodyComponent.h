#pragma once

#include "core/Core.h"
#include "component/Component.h"
#include "component/ComponentSerializer.h"
#include "serialize/CerealHelpers.h"

struct EngineAPI WaterBodyComponent : public Component
{
	WaterBodyComponent() = default;

	static void attachToEntity(std::shared_ptr<Component> c, Entity entityHandler, Scene& scene)
	{
		attachSimple<WaterBodyComponent>(c, entityHandler);
	}

	Entity nestedImpl = Entity::EmptyEntity;
};

REGISTER_COMPONENT(WaterBodyComponent)