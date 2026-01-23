#pragma once

#include "component/Component.h"
#include "component/ComponentSerializer.h"

struct EngineAPI SkyboxComponent : public Component
{
	SkyboxComponent() = default;

	SkyboxComponent(AssetHandle<Texture> skyboxImage);

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(originalImage);

	}

	void setSkybox(AssetHandle<Texture> image);

	void build();


	AssetHandle<Texture> originalImage;
	ResourceWrapper<Texture> cubemapIBL;
	ResourceWrapper<Texture> cubemap;
	static void attachToEntity(std::shared_ptr<Component>, Entity, Scene&);
};

REGISTER_COMPONENT(SkyboxComponent)