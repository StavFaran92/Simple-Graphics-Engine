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
		SERIALIZED_MEMBER(m_irradianceMap);
		SERIALIZED_MEMBER(m_prefilterEnvMap);
		SERIALIZED_MEMBER(m_cubemap);
		SERIALIZED_MEMBER(m_isBuilt);

	}

	void setSkybox(AssetHandle<TextureAsset> image);

	void build();

	void resolve(ResourceWrapper<Scene>& scene) override;
	void postLoad(ResourceWrapper<Scene>& scene) override;


	AssetHandle<TextureAsset> originalImage;
	ResourceWrapper<Texture> cubemapIBL;
	//ResourceWrapper<Texture> cubemap;
	AssetHandle<TextureAsset> m_prefilterEnvMap;

	//AssetHandle<TextureAsset>

	ResourceWrapper<Scene> m_scene = nullptr;

	bool m_isBuilt = false;
	AssetHandle<TextureAsset> m_cubemap;
	AssetHandle<TextureAsset> m_irradianceMap;
};

REGISTER_COMPONENT(SkyboxComponent)