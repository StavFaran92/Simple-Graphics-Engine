#pragma once

#include "component/Component.h"
#include "component/ComponentSerializer.h"
#include "texture/Texture.h"

struct EngineAPI SkyboxComponent : public Component
{
	SkyboxComponent() = default;

	SkyboxComponent(AssetHandle<TextureAsset> skyboxImage);

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(originalImage);

	}

	void setSkybox(AssetHandle<TextureAsset> image);

	void build();


	AssetHandle<TextureAsset> originalImage;
	ResourceWrapper<Texture> cubemapIBL;
	ResourceWrapper<Texture> cubemap;
	static void attachToEntity(std::shared_ptr<Component>, Entity, ResourceWrapper<Scene>&);
};

REGISTER_COMPONENT(SkyboxComponent)