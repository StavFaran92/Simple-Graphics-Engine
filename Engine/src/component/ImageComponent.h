#pragma once

#include "component/Component.h"
#include "component/ComponentSerializer.h"

struct EngineAPI ImageComponent : public Component
{
	ImageComponent() = default;

	ImageComponent(ResourceWrapper<Texture> image) : image(image) {}

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

	ResourceWrapper<Texture> image;
	static void attachToEntity(std::shared_ptr<Component> c, Entity entityHandler, Scene& scene)
	{
		(void)scene;
		attachSimple<ImageComponent>(c, entityHandler);
	}

};

REGISTER_COMPONENT(ImageComponent)