#pragma once

#include "component/Component.h"
#include "component/ComponentSerializer.h"

struct EngineAPI RenderableComponent : public Component
{
	RenderableComponent() = default;

	static void attachToEntity(std::shared_ptr<Component> c, Entity entityHandler, ResourceWrapper<Scene>& scene)
	{
		(void)scene;
		attachSimple<RenderableComponent>(c, entityHandler);
	}

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(renderTechnique);
	}

	enum class RenderTechnique : int
	{
		Forward,
		Deferred
	};

	RenderTechnique renderTechnique = RenderTechnique::Deferred;
};

REGISTER_COMPONENT(RenderableComponent)