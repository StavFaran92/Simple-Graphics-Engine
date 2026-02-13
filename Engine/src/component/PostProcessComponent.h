#pragma once

#include "component/Component.h"
#include "component/ComponentSerializer.h"
#include "memory/AssetHandle.h"
#include "render/Shader.h"

class PostProcessComponent : public Component
{
public:

	AssetHandle<ShaderAsset> shader;

	static void attachToEntity(std::shared_ptr<Component> c, Entity entityHandler, Scene& scene)
	{
		attachSimple<PostProcessComponent>(c, entityHandler);
	}

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(shader);
	}
};

REGISTER_COMPONENT(PostProcessComponent)