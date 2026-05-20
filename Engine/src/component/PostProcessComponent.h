#pragma once

#include "component/Component.h"
#include "component/ComponentSerializer.h"
#include "memory/AssetRef.h"
#include "render/Shader.h"

class PostProcessComponent : public Component
{
public:
	std::string getName() override { return "PostProcessComponent"; }

	ShaderAssetRef shader;

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZE_COMPONENT_BASE;
		SERIALIZED_MEMBER(shader);
	}
};

REGISTER_COMPONENT(PostProcessComponent)