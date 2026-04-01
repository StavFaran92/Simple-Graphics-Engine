#pragma once

#include "component/Component.h"
#include "component/ComponentSerializer.h"
#include "texture/Texture.h"

class Scene;

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

	void resolve(ResourceWrapper<Scene>& scene) override;


	AssetHandle<TextureAsset> originalImage;
	ResourceWrapper<Texture> cubemapIBL;
	ResourceWrapper<Texture> cubemap;

	ResourceWrapper<Scene> m_scene = nullptr;
};

REGISTER_COMPONENT(SkyboxComponent)