#pragma once

#include "component/Component.h"
#include "component/ComponentSerializer.h"

class PostProcessComponent : public Component
{
public:

	AssetWrapper<Shader> shader;

	static void attachToEntity(std::shared_ptr<Component> c, Entity entityHandler, Scene& scene)
	{
		attachSimple<PostProcessComponent>(c, entityHandler);
	}
};

REGISTER_COMPONENT(PostProcessComponent)