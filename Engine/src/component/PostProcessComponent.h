#pragma once

#include "component/Component.h"
#include "component/ComponentSerializer.h"
#include "memory/AssetHandle.h"
#include "render/Shader.h"

class PostProcessComponent : public Component
{
public:

	AssetHandle<ShaderAsset> shader;

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(shader);
	}
};

REGISTER_COMPONENT(PostProcessComponent)