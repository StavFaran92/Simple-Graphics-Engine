#pragma once

#include "component/Component.h"
#include "component/ComponentSerializer.h"

struct EngineAPI SkyboxComponent : public Component
{
	SkyboxComponent() = default;

	SkyboxComponent(ResourceWrapper<Texture> skyboxImage);

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(originalImage);

	}

	void setSkybox(ResourceWrapper<Texture> image);

	void build();


	ResourceWrapper<Texture> originalImage;
	ResourceWrapper<Texture> cubemap;
	static void attachToEntity(std::shared_ptr<Component>, Entity, Scene&);
};

REGISTER_COMPONENT(SkyboxComponent)