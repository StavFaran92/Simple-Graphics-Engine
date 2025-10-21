#pragma once

#include "component/Component.h"
#include "component/ComponentSerializer.h"

struct EngineAPI SkyboxComponent : public Component
{
	SkyboxComponent() = default;

	SkyboxComponent(AssetWrapper<Texture> skyboxImage);

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(originalImage);

	}

	void setSkybox(AssetWrapper<Texture> image);

	void build();


	AssetWrapper<Texture> originalImage;
	ResourceWrapper<Texture> cubemap;
	static void attachToEntity(std::shared_ptr<Component>, Entity, Scene&);
};

REGISTER_COMPONENT(SkyboxComponent)