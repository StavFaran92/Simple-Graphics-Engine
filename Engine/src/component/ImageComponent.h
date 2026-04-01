#pragma once

#include "component/Component.h"
#include "component/ComponentSerializer.h"

struct EngineAPI ImageComponent : public Component
{
	ImageComponent() = default;

	ImageComponent(AssetHandle<TextureAsset> image) : image(image) {}

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(image);
		SERIALIZED_MEMBER(size);
		SERIALIZED_MEMBER(position);
		SERIALIZED_MEMBER(rotate);
	}

	glm::vec2 size;
	glm::vec2 position;
	float rotate = 0;

	AssetHandle<TextureAsset> image;
};

REGISTER_COMPONENT(ImageComponent)